import copy

class Grid:
    def __init__(self) -> None:
        self.grid = []
    
class Cell:
    def __init__(self) -> None:
        self.policy = ''
        self.value = 0
        self.is_obstacle = False
        self.is_goal = False

class Problem:
    def __init__(self) -> None:
        self.rows = 0
        self.columns = 0
        self.obstacle_states = []
        self.goal_states = {}
        self.reward = 0
        self.action_noise = []
        self.gamma = 0
        self.epsilon = 0
        self.iteration = 0
    
    def CreateGrid(self):
        grid = Grid()
        for i in range(self.rows):
            row = []
            for j in range(self.columns):
                cell = Cell()
                if (i,j) in self.obstacle_states:
                    cell.is_obstacle = True
                elif (i,j) in self.goal_states:
                    cell.is_goal = True
                    cell.value = self.goal_states[(i,j)]
                row.append(cell)
            grid.grid.append(row)
        return grid

def CreateProblem(problem_file_name):
    problem = Problem()
    with open(problem_file_name) as file:
        lines = file.readlines()
    # get row and column numbers
    problem.rows = int(lines[1][:lines[1].find(' ')])
    problem.columns = int(lines[1][lines[1].find(' '):lines[1].find('\n')])
    # get obstacle states and store them as tuple
    obstacles = lines[3].split('|')
    for obstacle in obstacles:
        problem.obstacle_states.append(eval(obstacle))
    # get goal states and store them in dictionary as tuple: float
    goals = lines[5].split('|')
    for goal in goals:
        key = eval(goal[:goal.find(':')])
        value = eval(goal[goal.find(':')+1:])
        problem.goal_states[key] = value
    # get reward
    problem.reward = eval(lines[7])
    # get noise
    problem.action_noise.append(eval(lines[9]))
    problem.action_noise.append(eval(lines[10]))
    problem.action_noise.append(eval(lines[11]))
    # get gamma
    problem.gamma = eval(lines[13])
    # get epsilon
    problem.epsilon = eval(lines[15])
    # get iteration
    problem.iteration = eval(lines[17])
    return problem

actions = {'>': (0,1), 'V': (1,0), '<': (0,-1), '^': (-1,0)}

# returns probability of accessing state 's_' from 's' by taking action 'a'
def T(s, a, s_, problem: Problem):
    global actions
    if AddTuples(s, actions[a]) == s_:
        return problem.action_noise[0]
    elif AddTuples(s, actions[RotateAction(a, False)]) == s_:
        return problem.action_noise[1]
    elif AddTuples(s, actions[RotateAction(a, True)]) == s_:
        return problem.action_noise[2]

# returns reward value of reaching state 's_' from 's' by taking action 'a'
def R(s, a, s_, problem: Problem, grid: Grid):
    return problem.reward

def RotateAction(action, clockwise: bool):
    global actions
    action_list = list(actions.keys())
    index = action_list.index(action)
    if clockwise:
        index += 1
    else:
        index -= 1
    index %= len(action_list)
    return action_list[index]

def AddTuples(a, b):
    return tuple(map(sum,zip(a, b)))

def IsValid(state, grid: Grid):
    if state[0] < 0 or state[0] >= len(grid.grid) or state[1] < 0 or state[1] >= len(grid.grid[0]):
        return False
    elif grid.grid[state[0]][state[1]].is_obstacle:
        return False
    else:
        return True

def PossibleStates(state, action, grid):
    global actions
    state_normal = AddTuples(state, actions[action])
    state_cw = AddTuples(state, actions[RotateAction(action, True)])
    state_ccw = AddTuples(state, actions[RotateAction(action, False)])
    result = []
    result.append(state_normal)
    result.append(state_cw)
    result.append(state_ccw)
    return result

def Valuate(state, action, state_, V_p, problem, grid: Grid):
    if not IsValid(state_, grid):
        #print("{} * [{} + {}*{}]".format(T(state, action, state_, problem), R(state, action, state_, problem, grid), problem.gamma, V_p[(state[0], state[1])]))
        value = T(state, action, state_, problem) * (R(state, action, state_, problem, grid) + problem.gamma * V_p[(state[0], state[1])])
    else:
        #print("{} * [{} + {}*{}]".format(T(state, action, state_, problem), R(state, action, state_, problem, grid), problem.gamma, V_p[(state_[0], state_[1])]))
        value = T(state, action, state_, problem) * (R(state, action, state_, problem, grid) + problem.gamma * V_p[(state_[0], state_[1])])
    return value

def ValueIteration(problem: Problem, grid: Grid):
    global actions
    V_p = {} # Values of previous iteration
    V_c = {} # Values of current iteration
    # instantiate V_0
    for i in range(problem.rows):
        for j in range(problem.columns):
            V_p[(i,j)] = 0
            V_c[(i,j)] = 0

    while True:
        # update V_c values
        for row in range(problem.rows):
            for col in range(problem.columns):
                state = (row, col)
                if grid.grid[row][col].is_obstacle:
                    continue
                elif grid.grid[row][col].is_goal:
                    V_c[(row,col)] = grid.grid[row][col].value
                else:
                    max_val = float('-inf')
                    max_action = ''
                    for action in list(actions.keys()):
                        val = 0
                        for state_ in PossibleStates(state, action, grid):
                            val += Valuate(state, action, state_, V_p, problem, grid)
                        if val >= max_val:
                            max_val = val
                            max_action = action
                    V_c[(row,col)] = max_val
                    grid.grid[row][col].policy = max_action
        # Write new values to grid
        for r in range(problem.rows):
            for c in range(problem.columns):
                grid.grid[r][c].value = V_c[(r,c)]
        # check whether the values are the same as previous ones
        same = True
        for r in range(problem.rows):
            for c in range(problem.columns):
                if abs(V_c[(r,c)] - V_p[(r,c)]) > problem.epsilon:
                    same = False
        if same:
            break
        # Update previous values dict
        V_p = copy.deepcopy(V_c)
    return grid

def PolicyIteration(problem: Problem, grid: Grid):
    global actions
    V_p = {} # Values of previous iteration
    V_c = {} # Values of current iteration
    # instantiate V_0
    for i in range(problem.rows):
        for j in range(problem.columns):
            V_p[(i,j)] = 0
            V_c[(i,j)] = 0
    initial_action = list(actions.keys())[0]

    # iterate policy evaluation
    for iteration in range(problem.iteration):
        # update V_c values
        for row in range(problem.rows):
            for col in range(problem.columns):
                state = (row, col)
                if grid.grid[row][col].is_obstacle:
                    continue
                elif grid.grid[row][col].is_goal:
                    V_c[(row,col)] = grid.grid[row][col].value
                else:
                    val = 0
                    for state_ in PossibleStates(state, initial_action, grid):
                        val += Valuate(state, initial_action, state_, V_p, problem, grid)
                    V_c[(row,col)] = val
                    grid.grid[row][col].policy = initial_action
        # Write new values to grid
        for r in range(problem.rows):
            for c in range(problem.columns):
                grid.grid[r][c].value = V_c[(r,c)]
        
        # check whether the values are the same as previous ones
        # same = True
        # for r in range(problem.rows):
        #     for c in range(problem.columns):
        #         if abs(V_c[(r,c)] - V_p[(r,c)]) > problem.epsilon:
        #             same = False
        # if same:
        #     break
        # Update previous values dict
        V_p = copy.deepcopy(V_c)
        
    #print(V_c, '\n')

    P_p = {} # previous policies
    P_c = {} # current policies
    for i in range(problem.rows):
        for j in range(problem.columns):
            P_p[(i,j)] = initial_action
    P_c = copy.deepcopy(P_p)

    # Policy improvement
    for k in range(problem.iteration):
        #print("Iteration: ", '\n')
        for row_ in range(problem.rows):
            for col_ in range(problem.columns):
                state = (row_, col_)
                #print(state , ":")
                if grid.grid[row_][col_].is_obstacle:
                    continue
                elif grid.grid[row_][col_].is_goal:
                    continue
                else:
                    max_val = float('-inf')
                    max_policy = initial_action
                    for action in list(actions.keys()):
                        #print("Action: ", action)
                        val = 0
                        for state_ in PossibleStates(state, action, grid):
                            val += Valuate(state, action, state_, V_p, problem, grid)
                            #print("State_: ", state_, " Val: ", val)
                        if val > max_val:
                            max_val = val
                            max_policy = action
                    #print("Max: ", max_val, max_policy)
                    V_c[(row_,col_)] = max_val
                    P_c[(row_,col_)] = max_policy

        # check whether the policies are the same as previous ones
        # same = True
        # for r in range(problem.rows):
        #     for c in range(problem.columns):
        #         if V_p[(r,c)] != V_c[(r,c)]:
        #             same = False
        # if same:
        #     break
        # Write new values and policies to grid
        for r in range(problem.rows):
            for c in range(problem.columns):
                grid.grid[r][c].value = V_c[(r,c)]
                grid.grid[r][c].policy = P_c[(r,c)]
        # Update previous values and policies dicts 
        V_p = copy.deepcopy(V_c)
        P_p = copy.deepcopy(P_c)
    return grid

def SolveMDP(method_name, problem_file_name):
    problem = CreateProblem(problem_file_name)
    grid = problem.CreateGrid()

    if method_name == "ValueIteration":
        grid = ValueIteration(problem, grid)
    elif method_name == "PolicyIteration":
        grid = PolicyIteration(problem, grid)

    values = {}
    policies = {}
    for row in range(len(grid.grid)):
        for col in range(len(grid.grid[row])):
            cell = grid.grid[row][col]
            values[(row, col)] = cell.value
            if not cell.is_obstacle and not cell.is_goal:
                policies[(row, col)] = cell.policy
    
    return values, policies
            
