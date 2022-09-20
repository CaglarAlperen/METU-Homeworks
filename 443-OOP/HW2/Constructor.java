import java.util.Random;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.DoubleStream;

public class Constructor implements Runnable
{   
    private static Lock lock = new ReentrantLock();
    private static Condition ingot_loaded = lock.newCondition();
    private static Condition ingot_used = lock.newCondition();

    private int id, interval, capacity, type, storage, active_transporters, needed_ingots;

    public Constructor(int id, int interval, int capacity, int type)
    {
        this.id = id;
        this.interval = interval;
        this.capacity = capacity;
        this.type = type;
        this.active_transporters = 0;
        this.storage = 0;
        this.needed_ingots = (type == 1) ? 1 : 2;


        //System.out.println("Constructor created!");
    }

    public void run()
    {        
        HW2Logger.getInstance().Log(0, 0, 0, id, Action.CONSTRUCTOR_CREATED);
        while (active_transporters > 0 || storage >= needed_ingots)
        {
            System.out.println("Constructor working, storage: " + storage);
            WaitCanProduce();
            HW2Logger.getInstance().Log(0, 0, 0, id, Action.CONSTRUCTOR_STARTED);
            Sleep();
            HW2Logger.getInstance().Log(0, 0, 0, id, Action.CONSTRUCTOR_FINISHED);
            ConstructorProduced();
        }
        HW2Logger.getInstance().Log(0, 0, 0, id, Action.CONSTRUCTOR_STOPPED);
    }

    private void WaitCanProduce()
    {
        lock.lock();

        try {
            while (storage < needed_ingots)
            {
                System.out.println("Constructor has not enough ingots");
                ingot_loaded.await();
                System.out.println("Constructor has enough ingot");
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        storage -= (type == 1) ? 1 : 2;

        System.out.println("Constructor producing, storage: " + storage);

        lock.unlock();
    }

    private void Sleep()
    {
        Random random = new Random(System.currentTimeMillis());
        DoubleStream stream;
        stream = random.doubles(1, interval-interval*0.01, interval+interval*0.02);
        try {
            Thread.sleep((long) stream.findFirst().getAsDouble());
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void ConstructorProduced()
    {
        lock.lock();

        System.out.println("Constructor produced");

        ingot_used.signal();

        lock.unlock();
    }

    // Called by transporter
    public void IngotArrived()
    {
        lock.lock();

        try {
            while (storage >= capacity)
            {
                System.out.println("Constructor has to capacity to coming ingot");
                ingot_used.await();
                System.out.println("Constuctor have capacity");
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        storage++;
        ingot_loaded.signal();

        System.out.println("Constructor ingot arrived, storage: " + storage);

        lock.unlock();
    }

    public void AddTransporter()
    {
        lock.lock();

        active_transporters++;

        lock.unlock();
    }

    public void TransporterQuit()
    {
        lock.lock();

        active_transporters--;

        lock.unlock();
    }
}
