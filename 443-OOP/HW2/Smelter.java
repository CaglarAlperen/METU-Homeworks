import java.util.Random;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.DoubleStream;

public class Smelter implements Runnable
{
    private static Lock lock = new ReentrantLock();
    private static Condition ore_arrived = lock.newCondition();
    private static Condition ingot_producing = lock.newCondition();
    private static Condition ingot_produced = lock.newCondition();
    private static Condition ingot_loaded = lock.newCondition();

    private int id, interval, in_capacity, in_storage, out_capacity, out_storage, type, active_transporters, reserved, t_reserved;

    public Smelter(int id, int interval, int in_capacity, int out_capacity, int type)
    {
        this.id = id;
        this.interval = interval;
        this.in_capacity = in_capacity;
        this.out_capacity = out_capacity;
        this.type = type;
        this.active_transporters = 0;
        this.in_storage = 0;
        this.out_storage = 0;
        this.reserved = 0;
        this.t_reserved = 0;


        //System.out.println("Smelter created.");
    }

    public void run()
    {         
        HW2Logger.getInstance().Log(0, id, 0, 0, Action.SMELTER_CREATED);
        while (active_transporters > 0 || in_storage > type)
        {
            System.out.println("Smelter is working");
            WaitCanProduce();
            HW2Logger.getInstance().Log(0, id, 0, 0, Action.SMELTER_STARTED);
            Sleep();
            HW2Logger.getInstance().Log(0, id, 0, 0, Action.SMELTER_FINISHED);
            IngotProduced();
        }
        SmelterStopped();
        HW2Logger.getInstance().Log(0, id, 0, 0, Action.SMELTER_STOPPED);
    }

    private void WaitCanProduce()
    {
        lock.lock();

        try {
            while (out_storage + reserved >= out_capacity)
            {
                System.out.println("Smelter waiting, capacity full, out_storage: " + out_storage + " reserved: " + reserved + " out_capacity: " + out_capacity);
                ingot_loaded.await();
                System.out.println("Smelter ingot taken, out_storage: " + out_storage);
            }
            
            while (in_storage <= type)
            {
                System.out.println("Smelter dont have enough ore, in_storage: " + in_storage);
                ore_arrived.await();
                System.out.println("Smelter gote some ore, storage: " + in_storage);
            }

        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // Use 1 or 2 ores according to type and reserve storage for 
        // the next ingot will be produced
        in_storage -= type+1;
        reserved++;

        System.out.println("Smelter starting to produce ingot, in_storage: " + in_storage + " reserved: " + reserved);

        ingot_producing.signal();

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

    private void IngotProduced()
    {
        lock.lock();

        // for (Transporter transporter : Agents.getInstance().GetTransporters()) 
        // {
        //     if (transporter.GetSSmelterID() == id)
        //     {
        //         transporter.IngotReady();
        //     }
        // }

        reserved--;
        out_storage++;

        System.out.println("Smelter produced an ingot, out_storage: " + out_storage + " reserved: " + reserved);

        ingot_produced.signal();

        lock.unlock();
    }

    private void SmelterStopped()
    {
        lock.lock();

        System.out.println("Smelter stopped!");

        for (Transporter transporter : Agents.getInstance().GetTransporters()) 
        {
            if (transporter.GetSSmelterID() == id)
            {
                transporter.SmelterStopped();
            }
        }

        lock.unlock();
    }

    public void OreArrived()
    {
        lock.lock();

        try {
            while (in_storage >= in_capacity)
            {
                System.out.println("Smelter ore arrived no capacity, in_storage: " + in_storage + " in_capacity: " + in_capacity);
                ingot_producing.await();
                System.out.println("Smelter has capacity, in_storage: " + in_storage + " in_capacity: " + in_capacity);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ore_arrived.signal();
        in_storage++;

        System.out.println("Smelter ore in storage, in_stroage: " + in_storage + " in_capacity: " + in_capacity);

        lock.unlock();
    }

    public void IngotLoaded()
    {
        lock.lock();

        t_reserved++;

        try {
            while (out_storage <= 0)
            {
                System.out.println("Smelter there is no ingot to load");
                ingot_produced.await();
                System.out.println("Smelter ingot loaded");
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ingot_loaded.signal();
        out_storage--;
        t_reserved--;

        System.out.println("Smelter, out_storage: " + out_storage);

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

    public int GetOutStorage()
    {
        return out_storage - t_reserved;
    }
}
