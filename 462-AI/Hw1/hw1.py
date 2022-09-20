
class Node:
    def __init__(self, name, parent, depth, path_cost):
        self.name = name
        self.parent = parent
        self.depth = depth
        self.path_cost = path_cost

def Solution(node, processed, ucs=False):
    path = []
    depth = node.depth
    path_cost = node.path_cost
    while node is not None:
        path.insert(0, node.name)
        node = node.parent
    if ucs == False:
        return (path, processed, depth)
    else:
        return (path, processed, depth, path_cost)

def InsertSorted(fringe, node):
    for i in range(len(fringe)):
        if node.path_cost < fringe[i].path_cost:
            fringe.insert(i, node)
            return fringe
    else:
        fringe.append(node)
        return fringe

def BFS(start, target, problem):
    fringe = [Node(start, None, 0, 0)]
    processed = []

    while len(fringe) > 0:
        node = fringe.pop(0)
        processed.append(node.name)

        if node.name == target:
            return Solution(node, processed)
        
        for edge in problem:
            if edge[0] == node.name:
                fringe.append(Node(edge[1], node, node.depth+1, 0))
            elif edge[1] == node.name:
                fringe.append(Node(edge[0], node, node.depth+1, 0))

    return None        

def DLS(start, target, problem, maximum_depth_limit):
    fringe = [Node(start, None, 0, 0)]
    processed = []

    while len(fringe) > 0:
        node = fringe.pop(0)
        processed.append(node.name)

        if node.name == target:
            return Solution(node, processed)

        if node.depth == maximum_depth_limit:
            continue

        for edge in problem:
            if edge[0] == node.name:
                fringe.insert(0, Node(edge[1], node, node.depth+1, 0))
            elif edge[1] == node.name:
                fringe.insert(0, Node(edge[0], node, node.depth+1, 0))
                
    return None

def IDDFS(start, target, problem, maximum_depth_limit):
    for i in range(maximum_depth_limit+1):
        result = DLS(start, target, problem, i)
        if result is not None:
            return result
    else:
        return None

def UCS(start, target, problem):
    fringe = [Node(start, None, 0, 0)]
    processed = []

    while len(fringe) > 0:
        node = fringe.pop(0)
        processed.append(node.name)

        if node.name == target:
            return Solution(node, processed, True)

        for edge in problem:
            if edge[0] == node.name:
                fringe = InsertSorted(fringe, Node(edge[1], node, node.depth+1, node.path_cost+edge[2]))
            elif edge[1] == node.name:
                fringe = InsertSorted(fringe, Node(edge[0], node, node.depth+1, node.path_cost+edge[2]))
    
    return None

def UnInformedSearch(method_name, problem_file_name, maximum_depth_limit):
    
    with open(problem_file_name) as f:
        start = f.readline().strip('\n')
        target = f.readline().strip('\n')
        problem = f.readlines()
    for i in range(len(problem)):
        problem[i] = problem[i].strip('\n')
        problem[i] = problem[i].split(' ')
        problem[i][2] = int(problem[i][2])

    if method_name == "BFS":
        return BFS(start, target, problem)
    elif method_name == "DLS":
        return DLS(start, target, problem, maximum_depth_limit)
    elif method_name == "IDDFS":
        return IDDFS(start, target, problem, maximum_depth_limit)
    elif method_name == "UCS":
        return UCS(start, target, problem)
    else:
        raise Exception("Please enter a valid method name ('BFS','DLS','IDDFS','UCS')")


    
