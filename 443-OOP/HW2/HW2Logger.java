import java.io.PrintStream;


public class HW2Logger{
    private static final HW2Logger _inst = new HW2Logger();
    public static HW2Logger getInstance() {
        return _inst;
    }
    private long startTime;
    private final PrintStream out = System.out;
    
    private long getTimeDifference() {
        return System.nanoTime()-startTime;
    }

    public void Initialize() {
        this.startTime = System.nanoTime();
    }

    public void Log(int minerID, int smelterID, int transporterID, int constructorID, Action action) {
        long currentThreadID = Thread.currentThread().getId();
        long timestamp = this.getTimeDifference();

        System.out.printf("ThreadID: %d Miner: %d Smelter: %d Transporter: %d Constructor: %d Action: %s TimeStamp: %f ms %n",
                currentThreadID, minerID, smelterID, transporterID, constructorID, action.toString(), timestamp / 1000000.0);
        System.out.flush();
    }
}
