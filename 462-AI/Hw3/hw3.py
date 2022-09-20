# I made processed list global in order to fill in recursive function
processed = []

# returns successors of game tree state
def GetChildren(state, edges):
    childs = []
    for edge in edges:
        if edge[0] == state:
            childs.append(edge)
    return childs

# returns successors of tic-tac-toe game state according to player
def GetChildrenTicTacToe(state, sign):
    children = []
    for i in range(9):
        if state[i] == ' ':
            children.append(state[:i] + sign + state[i+1:])
    return children

def IsTerminal(state: str):
    # rows
    if state[:3] == "XXX" or state[:3] == "OOO" or state[3:6] == "XXX" or state[3:6] == "OOO" or state[6:] == "XXX" or state[6:] == "OOO":
        return True
    # columns
    elif state[0] == state[3] == state[6] != ' ' or state[1] == state[4] == state[7] != ' ' or state[2] == state[5] == state[8] != ' ':
        return True
    # diagonals
    elif state[0] == state[4] == state[8] != ' ' or state[2] == state[4] == state[6] != ' ':
        return True
    # full
    elif state.find(' ') == -1:
        return True
    else:
        return False

# retruns taken action in order to get child from state
def GetAction(state, child):
    for i in range(9):
        if state[i] != child[i]:
            return (i%3, i//3)

def CalcUtility(state, depth):
    # wins
    if state[:3] == "XXX" or state[3:6] == "XXX" or state[6:] == "XXX" or state[0] == state[3] == state[6] == "X" or state[1] == state[4] == state[7] == "X" or state[2] == state[5] == state[8] == "X" or state[0] == state[4] == state[8] == "X" or state[2] == state[4] == state[6] == "X":
        return 5 - 0.01*(depth-1)
    # loses
    elif state[:3] == "OOO" or state[3:6] == "OOO" or state[6:] == "OOO" or state[0] == state[3] == state[6] == "O" or state[1] == state[4] == state[7] == "O" or state[2] == state[5] == state[8] == "O" or state[0] == state[4] == state[8] == "O" or state[2] == state[4] == state[6] == "O":
        return -5
    else:
        return 0

def GameTreeMinimax(player_type, state, edges, leaves):
    global processed
    action = ''

    if state in leaves:
        return (leaves[state], action)

    if player_type == "MAX":
        max_val = float('-inf')
        children = GetChildren(state, edges)
        for edge in children:
            processed.append(edge[1])
            val = GameTreeMinimax("MIN", edge[1], edges, leaves)
            if val[0] > max_val:
                max_val = val[0]
                action = edge[2]
        return (max_val, action)

    elif player_type == "MIN":
        min_val = float('inf')
        children = GetChildren(state, edges)
        for edge in children:
            processed.append(edge[1])
            val = GameTreeMinimax("MAX", edge[1], edges, leaves)
            if val[0] < min_val:
                min_val = val[0]
                action = edge[2]
        return (min_val, action)
        
def GameTreeAlphaBeta(player_type, state, alpha, beta, edges, leaves):
    global processed
    action = ''

    if state in leaves:
        return (leaves[state], action)

    if player_type == "MAX":
        max_val = float('-inf')
        childs = GetChildren(state, edges)
        for edge in childs:
            processed.append(edge[1])
            value = GameTreeAlphaBeta("MIN", edge[1], alpha, beta, edges, leaves)
            if value[0] > max_val:
                max_val = value[0]
                action = edge[2]
            if max_val >= beta:
                return (max_val, action)
            alpha = max(alpha, max_val)
        return (max_val, action)

    elif player_type == "MIN":
        min_val = float('inf')
        childs = GetChildren(state, edges)
        for edge in childs:
            processed.append(edge[1])
            value = GameTreeAlphaBeta("MAX", edge[1], alpha, beta, edges, leaves)
            if value[0] < min_val:
                min_val = value[0]
                action = edge[2]
            if min_val <= alpha:
                return (min_val, action)
            beta = min(beta, min_val)
        return (min_val, action)

def TicTacToeMinimax(player_type, state, depth):
    global processed
    action = ''

    if IsTerminal(state):
        return (CalcUtility(state, depth), action)

    if player_type == "MAX":
        max_val = float('-inf')
        children = GetChildrenTicTacToe(state, 'X')
        for child in children:
            processed.append(child)
            val = TicTacToeMinimax("MIN", child, depth+1)
            if val[0] > max_val:
                max_val = val[0]
                action = GetAction(state, child)
        return (max_val, action)
    
    elif player_type == "MIN":
        min_val = float('inf')
        children = GetChildrenTicTacToe(state, 'O')
        for child in children:
            processed.append(child)
            val = TicTacToeMinimax("MAX", child, depth+1)
            if val[0] < min_val:
                min_val = val[0]
                action = GetAction(state, child)
        return (min_val, action)

def TicTacToeAlphaBeta(player_type, state, depth, alpha, beta):
    global processed
    action = ''

    if IsTerminal(state):
        return (CalcUtility(state, depth), action)
    
    if player_type == "MAX":
        max_val = float('-inf')
        children = GetChildrenTicTacToe(state, 'X')
        for child in children:
            processed.append(child)
            val = TicTacToeAlphaBeta("MIN", child, depth+1, alpha, beta)
            if val[0] > max_val:
                max_val = val[0]
                action = GetAction(state, child)
            if max_val >= beta:
                return (max_val, action)
            alpha = max(alpha, max_val)
        return (max_val, action)
    
    elif player_type == "MIN":
        min_val = float('inf')
        children = GetChildrenTicTacToe(state, 'O')
        for child in children:
            processed.append(child)
            val = TicTacToeAlphaBeta("MAX", child, depth+1, alpha, beta)
            if val[0] < min_val:
                min_val = val[0]
                action = GetAction(state, child)
            if min_val <= alpha:
                return (min_val, action)
            beta = min(beta, min_val)
        return (min_val, action)

def GameTree(method_name, player_type, start_state, edges, leaves):
    processed.clear()
    if method_name == "Minimax":
        result = GameTreeMinimax(player_type, start_state, edges, leaves)
    elif method_name == "AlphaBeta":
        alpha = float('-inf')
        beta = float('inf')
        result = GameTreeAlphaBeta(player_type, start_state, alpha, beta, edges, leaves)
    return (result[0], result[1], processed)

def TicTacToe(method_name, player_type, start_state):
    processed.clear()
    if method_name == "Minimax":
        result = TicTacToeMinimax(player_type, start_state, 0)
    elif method_name == "AlphaBeta":
        alpha = float('-inf')
        beta = float('inf')
        result = TicTacToeAlphaBeta(player_type, start_state, 0, alpha, beta)
    return (result[0], result[1], processed)

def SolveGame(method_name, problem_file_name, player_type):
    
    with open(problem_file_name, 'r') as file:
        lines = file.readlines()
        for i in range(len(lines)):
            lines[i] = lines[i].strip('\n')
            lines[i] = lines[i].strip('#')

        # If problem is game tree than read the file accordingly
        # and call the game tree method.
        if problem_file_name[0] == 'g':
            start_state = lines[0]
            edges = []
            leaves = {}
            for line in lines[1:]:
                find = line.find(':')
                if find == -1:
                    edges.append(line.split(' '))
                else:
                    leaves[line[:find]] = int(line[find+1:])
            return GameTree(method_name, player_type, start_state, edges, leaves)

        elif problem_file_name[0] == 't':
            start_state = ''.join(lines)
            return TicTacToe(method_name, player_type, start_state)

    
