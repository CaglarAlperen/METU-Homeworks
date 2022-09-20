# I used heuristicEight function to calculate heuristic value for 
# eight puzzle which counts misplaced numbers. 
# I used heuristicMaze function to calculate heuristic value for
# maze which calculates manhattan distance between state point
# and goal

actions = ["LEFT", "UP", "RIGHT", "DOWN"]

class Node:
    def __init__(self, state, previous, depth, path_cost, heuristic, last_action):
        self.state = state
        self.previous = previous
        self.depth = depth
        self.path_cost = path_cost
        self.heuristic = heuristic
        self.f = path_cost + heuristic
        self.last_action = last_action

def swap(string, index1, index2):
    l = list(string)
    (l[index1], l[index2]) = (l[index2], l[index1])
    return ''.join(l)

def ValidEight(action, state):
    index = state.find(' ')
    if action == "LEFT":
        return index % 3 != 0
    elif action == "UP":
        return index > 2
    elif action == "RIGHT":
        return index % 3 != 2
    elif action == "DOWN":
        return index < 6

def ValidMaze(action, state, maze):
    if action == "LEFT":
        state = (state[0]-1, state[1])
    elif action == "UP":
        state = (state[0], state[1]-1)
    elif action == "RIGHT":
        state = (state[0]+1, state[1])
    elif action == "DOWN":
        state = (state[0], state[1]+1)
    if state[0] < 0 or state[0] >= len(maze[0]) or state[1] < 0 or state[1] >= len(maze):
        return False
    elif maze[state[1]][state[0]] == '#':
        return False
    else:
        return True

def Solution(node, processed, action_name = False):
    path = []
    depth = node.depth
    path_cost = node.path_cost
    
    while node is not None:
        if action_name:
            if node.last_action is not None:
                path.insert(0, node.last_action)
        else:
            path.insert(0, node.state)
        node = node.previous
    return (path, processed, depth, path_cost)

def insertSortedUCS(fringe, node):
    for i in range(len(fringe)):
        if node.path_cost < fringe[i].path_cost:
            fringe.insert(i, node)
            return fringe
    else:
        fringe.append(node)
        return fringe

def insertSortedAStar(fringe, node):
    for i in range(len(fringe)):
        if node.f < fringe[i].f:
            fringe.insert(i, node)
            return fringe
    else:
        fringe.append(node)
        return fringe

def nextStateEight(action, state):
    index = state.find(' ')
    if action == "LEFT":
        state = swap(state, index, index-1)
    elif action == "UP":
        state = swap(state, index, index-3)
    elif action == "RIGHT":
        state = swap(state, index, index+1)
    elif action == "DOWN":
        state = swap(state, index, index+3)
    return state
    
def nextStateMaze(action, state):
    if action == "LEFT":
        return (state[0]-1, state[1])
    elif action == "UP":
        return (state[0], state[1]-1)
    elif action == "RIGHT":
        return (state[0]+1, state[1])
    elif action == "DOWN":
        return (state[0], state[1]+1)

def heuristicEight(state, goal):
    h = 0
    for i in range(len(state)):
        if state[i] != goal[i]:
            h += 1
    return h-1

def heuristicMaze(state, goal):
    return abs(goal[0] - state[0]) + abs(goal[1] - state[1])

def UCSeight(start, target):
    fringe = [Node(start, None, 0, 0, 0, None)]
    processed = []
    closed = set()

    while len(fringe) > 0:
        node = fringe.pop(0)

        if node.state == target:
            processed.append(node.state)
            return Solution(node, processed, True)
        
        if node.state not in closed:
            processed.append(node.state)
            closed.add(node.state)
            for action in actions:
                if ValidEight(action, node.state):
                    fringe = insertSortedUCS(fringe, Node(nextStateEight(action, node.state), node, node.depth+1, node.path_cost+1, 0, action))
    return None

def UCSmaze(start, target, maze):
    fringe = [Node(start, None, 0, 0, 0, None)]
    processed = []
    closed = set()

    while len(fringe) > 0:
        node = fringe.pop(0)

        if node.state == target:
            processed.append(node.state)
            return Solution(node, processed)
        
        if node.state not in closed:
            processed.append(node.state)
            closed.add(node.state)
            for action in actions:
                if ValidMaze(action, node.state, maze):
                    fringe = insertSortedUCS(fringe, Node(nextStateMaze(action, node.state), node, node.depth+1, node.path_cost+1, 0, action))

def AStareight(start, target):
    fringe = [Node(start, None, 0, 0, 0, None)]
    processed = []
    closed = set()

    while len(fringe) > 0:
        node = fringe.pop(0)

        if node.state == target:
            processed.append(node.state)
            return Solution(node, processed, True)
        
        if node.state not in closed:
            processed.append(node.state)
            closed.add(node.state)
            for action in actions:
                if ValidEight(action, node.state):
                    fringe = insertSortedAStar(fringe, Node(nextStateEight(action, node.state), node, node.depth+1, node.path_cost+1, heuristicEight(node.state, target), action))
    return None

def AStarmaze(start, target, maze):
    fringe = [Node(start, None, 0, 0, 0, None)]
    processed = []
    closed = set()

    while len(fringe) > 0:
        node = fringe.pop(0)

        if node.state == target:
            processed.append(node.state)
            return Solution(node, processed)

        if node.state not in closed:
            processed.append(node.state)
            closed.add(node.state)
            for action in actions:
                if ValidMaze(action, node.state, maze):
                    fringe = insertSortedAStar(fringe, Node(nextStateMaze(action, node.state), node, node.depth+1, node.path_cost+1, heuristicMaze(node.state, target), action))

def InformedSearch(method_name, problem_file_name):
    with open(problem_file_name) as f:
        lines = f.readlines()
        for i in range(len(lines)):
            lines[i] = lines[i].strip('\n')
        if problem_file_name[0] == 'e':
            eightpuzzlestart = (lines[0] + lines[1] + lines[2]).replace(' ', '')
            eightpuzzlestart = eightpuzzlestart.replace('0', ' ')
            eightpuzzlegoal = (lines[4] + lines[5] + lines[6]).replace(' ','')
            eightpuzzlegoal = eightpuzzlegoal.replace('0', ' ')
        elif problem_file_name[0] == 'm':
            mazestart = (int(lines[0][1:lines[0].find(',')]), int(lines[0][lines[0].find(',')+1:lines[0].find(')')]))
            mazegoal = (int(lines[1][1:lines[1].find(',')]), int(lines[1][lines[1].find(',')+1:lines[1].find(')')]))
            maze = lines[2:]
    
    if method_name == "UCS":
        if problem_file_name[0] == 'e':
            return UCSeight(eightpuzzlestart, eightpuzzlegoal)
        elif problem_file_name[0] == 'm':
            return UCSmaze(mazestart, mazegoal, maze)
    elif method_name == "AStar":
        if problem_file_name[0] == 'e':
            return AStareight(eightpuzzlestart, eightpuzzlegoal)
        elif problem_file_name[0] == 'm':
            return AStarmaze(mazestart, mazegoal, maze)
    