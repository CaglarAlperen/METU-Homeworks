import random 
import copy

#Student name & surname: Çağlar Alperen Tosun   
#Student ID: 2310530

class Problem:
    def __init__(self) -> None:
        self.grid = []
        self.rows = 0
        self.columns = 0
        self.obstacle_states = []
        self.goal_states = {}
        self.start_state = (0,0)
        self.reward = 0
        self.action_noise = []
        self.learning_rate = 0
        self.gamma = 0
        self.epsilon = 0
        self.episode_count = 0

def readFile(file_name):
    problem = Problem()
    with open(file_name) as file:
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
    # get start state
    problem.start_state = eval(lines[7])
    # get reward
    problem.reward = eval(lines[9])
    # get noise
    problem.action_noise.append(eval(lines[12]))
    problem.action_noise.append(eval(lines[11]))
    problem.action_noise.append(eval(lines[13]))
    # get learning rate
    problem.learning_rate = eval(lines[15])
    # get gamma
    problem.gamma = eval(lines[17])
    # get epsilon
    problem.epsilon = eval(lines[19])
    # get episode count
    problem.episode_count = eval(lines[21])

    return problem

actions = {'<': (0,-1), '^': (-1,0), '>': (0,1), 'V': (1,0)}

def tupleSum(t1, t2):
    return tuple(map(sum, zip(t1, t2)))

def chooseMax(U, state, problem):
    max_val = float('-inf')
    for action in actions.keys():
        next_state = nextState(action, state)
        if notValid(next_state, problem):
            continue
        if U[next_state] >= max_val:
            max_val = U[next_state]
    return max_val

def calculateUtility(U, s, s_, r, problem):
    return U[s] + problem.learning_rate * (r + problem.gamma * U[s_] - U[s])

def calculateUtilityQ(Q, s, a, s_, r, problem):
    return Q[(s,a)] + problem.learning_rate * (r + problem.gamma * qval(Q,s_) - Q[(s,a)])

def possibleActions(action):
    index = list(actions.keys()).index(action)
    index = (index-1) % 4
    result = []
    for i in range(3):
        result.append(list(actions.keys())[index])
        index = (index+1) % 4
    return result

def nextState(action, state):
    # print("Action:", action) 
    # print("State:", state)
    # print("Action value: ", actions[action])
    # print("Result: ", tupleSum(state, actions[action]))
    return tupleSum(state, actions[action])

def notValid(state, problem):
    # print("notValid")
    # print("state: ", state)
    # print("limits: ", problem.rows, problem.columns)
    if state[0] not in range(problem.rows) or state[1] not in range(problem.columns):
        return True
    if state in problem.obstacle_states:
        return True
    else:
        return False

def argmax(state, U, problem):
    max_val = float('-inf')
    max_arg = ''

    for action in actions:
        next_state = nextState(action, state)
        if notValid(next_state, problem):
            if U[state] > max_val:
                max_val = U[state]
                max_arg = action
        elif U[next_state] > max_val:
            max_val = U[next_state]
            max_arg = action
    return max_arg

def qmax(state, Q, problem):
    max_val = float('-inf')
    max_arg = ''

    for action in actions:
        if Q[(state, action)] > max_val:
            max_val = Q[(state, action)]
            max_arg = action
    return max_arg

# calculates utility value of a state
def qval(Q, state):
    max_val = float('-inf')
    for action in actions:
        if Q[(state, action)] > max_val:
            max_val = Q[(state, action)]
    return max_val

def TD0(problem: Problem):
    # initialize utility vector with zeros
    U = {}
    for i in range(problem.rows):
        for j in range(problem.columns):
            U[(i,j)] = 0
    
    for episode in range(problem.episode_count):
        state = problem.start_state
        while state not in problem.goal_states.keys():
            if random.random() <= problem.epsilon:
                action = list(actions.keys())[random.randint(0,3)]
            else:
                #action = argmax(U, state, problem.rows-1, problem.columns-1, problem.obstacle_states)
                action = argmax(state, U, problem)
                #print(action)
            
            possible_actions = possibleActions(action)
            weights = problem.action_noise
            taken_action = random.choices(possible_actions, weights)[0]

            next_state = nextState(taken_action, state)
            # print("NS: ", next_state)
            if notValid(next_state, problem):
                # print("Not valid")
                next_state = state

            if next_state in problem.goal_states.keys():
                reward = problem.reward + problem.goal_states[next_state]
                U[state] = calculateUtility(U, state, next_state, reward, problem)
                break
            else:
                # print("Else")
                reward = problem.reward
                U[state] = calculateUtility(U, state, next_state, reward, problem)
                # print(state, next_state)
                
                # print(state, next_state)
            state = next_state
            # print(U, '\n')
        
    for goal in problem.goal_states.keys():
        U[goal] = problem.goal_states[goal]
    
    for obstacle in problem.obstacle_states:
        U.pop(obstacle)

    policy = {}
    for state in U.keys():
            policy[state] = argmax(state, U, problem)

    for goal in problem.goal_states.keys():
        policy.pop(goal)

    for utility in U.keys():
        U[utility] = round(U[utility],2)
    
    return U, policy

def qLearning(problem: Problem):
    # initialize utility table with zeros
    Q = {}
    for i in range(problem.rows):
        for j in range(problem.columns):
            for a in actions.keys():
                Q[((i,j),a)] = 0

    for episode in range(problem.episode_count):
        state = problem.start_state
        while state not in problem.goal_states.keys():
            if random.random() <= problem.epsilon:
                #print("Random")
                action = list(actions.keys())[random.randint(0,3)]
            else:
                action = qmax(state, Q, problem)
            
            #print("Action: ", action)
            possible_actions = possibleActions(action)
            weights = problem.action_noise
            taken_action = random.choices(possible_actions, weights)[0]
            #print("Taken action: ", taken_action)

            next_state = nextState(taken_action, state)
            #print("Next state: ", next_state)
            if notValid(next_state, problem):
                #print("Not valid")
                next_state = state
            
            if next_state in problem.goal_states.keys():
                #print("Goal")
                reward = problem.reward + problem.goal_states[next_state]
                Q[(state, action)] = calculateUtilityQ(Q, state, action, next_state, reward, problem)
                break
            else:
                reward = problem.reward
                Q[(state, action)] = calculateUtilityQ(Q, state, action, next_state, reward, problem)
            
            state = next_state

    U = {}
    for i in range(problem.rows):
        for j in range(problem.columns):
            U[(i,j)] = qval(Q, (i,j))

    for goal in problem.goal_states.keys():
        U[goal] = problem.goal_states[goal]

    for obstacle in problem.obstacle_states:
        U.pop(obstacle)

    policy = {}
    for state in U.keys():
            policy[state] = qmax(state, Q, problem)

    for goal in problem.goal_states.keys():
        policy.pop(goal)

    for utility in U.keys():
        U[utility] = round(U[utility],2)

    return U, policy

def SolveMDP(method_name, problem_file, seed = 123):
    random.seed(seed)
    
    problem = readFile(problem_file)

    if method_name == "TD(0)":
        U, policy = TD0(problem)
    elif method_name == "Q-learning":
        U, policy = qLearning(problem)

    return U, policy