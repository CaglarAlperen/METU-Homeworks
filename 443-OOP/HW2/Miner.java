import java.util.Random;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.DoubleStream;

public class Miner implements Runnable
{
    private static Lock lock = new ReentrantLock();
    private static Condition ore_produced = lock.newCondition();
    private static Condition ore_loaded = lock.newCondition();

    private int id, interval, capacity, storage, type, retire, reserved, t_reserved;

    public Miner(int id, int interval, int capacity, int type, int retire)
    {
        this.id = id;
        this.interval = interval;
        this.capacity = capacity;
        this.type = type;
        this.retire = retire;
        this.storage = 0;
        this.reserved = 0;
        this.t_reserved = 0;

        //System.out.println("Miner Created");
    }

    public void run()
    {
        HW2Logger.getInstance().Log(id, 0, 0, 0, Action.MINER_CREATED);
        while (retire > 0)
        {
            System.out.println("Miner working, retire: " + retire);
            WaitCanProduce();
            HW2Logger.getInstance().Log(id, 0, 0, 0, Action.MINER_STARTED);
            Sleep();
            HW2Logger.getInstance().Log(id, 0, 0, 0, Action.MINER_FINISHED);
            OreProcuced();
        }
        MinerStopped();
        HW2Logger.getInstance().Log(id, 0, 0, 0, Action.MINER_STOPPED);
    }

    private void WaitCanProduce()
    {
        lock.lock();

        try {
            while (storage + reserved >= capacity)
            {
                System.out.println("Miner waiting because capacity is full.");
                ore_loaded.await();
                System.out.println("Miner back to work.");
            }
            // Reserve a storage place for the next ore
            reserved++;
            retire--;

            System.out.println("Miner, reserved: " + reserved + "storage: " + storage);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

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

    private void OreProcuced()
    {
        lock.lock();

        // for (Transporter transporter : Agents.getInstance().GetTransporters()) 
        // {
        //     if (transporter.GetMinerID() == id)
        //     {
        //         transporter.OreReady();
        //     }
        // }

        storage++;
        reserved--;

        System.out.println("Miner produced an Ore, storage: " + storage + "reserved: " + reserved);

        ore_produced.signal();

        lock.unlock();
    }

    private void MinerStopped()
    {
        lock.lock();

        System.out.println("Miner stopped!");

        for (Transporter transporter : Agents.getInstance().GetTransporters()) 
        {
            if (transporter.GetMinerID() == id)
            {
                transporter.MinerStopped();
            }
        }

        lock.unlock();
    }

    public void OreLoaded()
    {
        lock.lock();

        t_reserved++;

        try {
            // If there is no ore in storage wait until one produced
            while (storage <= 0) 
            {
                System.out.println("Miner has no ore to load");
                ore_produced.await();
                System.out.println("Miner ore loaded");
            }

        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ore_loaded.signal();
        storage--;
        t_reserved--;

        System.out.println("Miner storage: " + storage);

        lock.unlock();
    }

    public int GetOutStorage()
    {
        return storage - t_reserved;
    }
}
