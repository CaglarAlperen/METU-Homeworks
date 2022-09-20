
class Problem:
    def __init__(self, states, start_probs, trans_probs, obs_probs, observations):
        self.states = states
        self.start_probs = start_probs
        self.trans_probs = trans_probs
        self.obs_probs = obs_probs
        self.observations = observations

def read_file(problem_file_name):
    with open(problem_file_name, "r") as f:
        lines = f.readlines()
    for i in range(len(lines)):
        lines[i] = lines[i].strip("\n")
    # get states
    states = lines[1].split("|")
    # get start_probs
    start_probs = {}
    pairs = lines[3].split("|")
    for pair in pairs:
        pair = pair.split(":")
        start_probs[pair[0]] = eval(pair[1])
    # get trans_probs
    trans_probs = {}
    pairs = lines[5].split("|")
    for pair in pairs:
        pair = pair.split(":")
        trans_probs[pair[0]] = eval(pair[1])
    # get obs_probs
    obs_probs = {}
    pairs = lines[7].split("|")
    for pair in pairs:
        pair = pair.split(":")
        obs_probs[pair[0]] = eval(pair[1])
    # get observations
    observations = lines[9].split("|")

    return Problem(states, start_probs, trans_probs, obs_probs, observations)
    
def P(state, t, m, problem: Problem):
    # calculates max value of state and returns
    # previous state of optimal path with value
    max_val = 0
    max_state = None
    for prev in problem.states:
        transition = prev+'-'+state
        evidence = state+'-'+problem.observations[t]
        val = m[prev][t-1] * problem.trans_probs[transition] * problem.obs_probs[evidence]
        if val > max_val:
            max_val = val
            max_state = prev
    return max_val, max_state

def viterbi(problem_file_name):
    problem = read_file(problem_file_name)
    
    # initialize dictionaries
    m = {}
    p = {}
    p_prev = {}
    for state in problem.states:
        m[state] = []
        p[state] = []
        p_prev[state] = []

    for t in range(0,len(problem.observations)):
        for state in problem.states:
            # calculate starting probabilities
            if t == 0:
                e = state+'-'+problem.observations[0]
                val = problem.start_probs[state] * problem.obs_probs[e]
                m[state].append(val)
                p[state].append(state)
                continue
            # val is the max value and prev is the
            # previous state of the optimal way
            val, prev = P(state, t, m, problem)
            m[state].append(val)
            p[state] = p_prev[prev]+[state]
        
        # copy path lists after each time iteration
        for s in problem.states:
            p_prev[s] = p[s]
    
    max_val = 0
    max_state = None
    for state in problem.states:
        if m[state][-1] > max_val:
            max_val = m[state][-1]
            max_state = state
    
    return p[max_state], max_val, m