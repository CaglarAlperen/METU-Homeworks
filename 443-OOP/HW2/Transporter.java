import java.util.Random;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.stream.DoubleStream;

public class Transporter implements Runnable
{
    private static Lock lock = new ReentrantLock();
    private static Condition source_produced = lock.newCondition();

    private int id, interval, s_miner, s_smelter, t_smelter, t_constructor;
    private boolean source_acive;
    private Runnable source, target;
    
    public Transporter(int id, int interval, int s_miner, int s_smelter, int t_smelter, int t_constructor)
    {
        this.id = id;
        this.interval = interval;
        this.s_miner = s_miner;
        this.s_smelter = s_smelter;
        this.t_smelter = t_smelter;
        this.t_constructor = t_constructor;
        this.source_acive = true;
        // Store references of source and target
        source = (s_miner != 0) ? Agents.getInstance().GetMinerAgent(s_miner) : 
                                  Agents.getInstance().GetSmelterAgent(s_smelter);
        target = (t_smelter != 0) ? Agents.getInstance().GetSmelterAgent(t_smelter) :
                                    Agents.getInstance().GetConstructorAgent(t_constructor);

        // Should update active transformer number of target
        if (t_smelter != 0)
            ((Smelter) target).AddTransporter();
        else if (t_constructor != 0)
            ((Constructor) target).AddTransporter();

        
        //System.out.println("Transporter created!");
    }

    public void run()
    {
        HW2Logger.getInstance().Log(0, 0, id, 0, Action.TRANSPORTER_CREATED);
        if (s_miner != 0)
            HW2Logger.getInstance().Log(s_miner, 0, id, 0, Action.TRANSPORTER_GO);
        else
            HW2Logger.getInstance().Log(0, s_smelter, id, 0, Action.TRANSPORTER_GO);
        
        while (SourceHasMaterial() || SourceIsActive())
        {
            System.out.println("Transporter working");
            Sleep();

            if (s_miner != 0)
                HW2Logger.getInstance().Log(s_miner, 0, id, 0, Action.TRANSPORTER_ARRIVE);
            else
                HW2Logger.getInstance().Log(0, s_smelter, id, 0, Action.TRANSPORTER_ARRIVE);

            WaitNextLoad();

            if (s_miner != 0)
                HW2Logger.getInstance().Log(s_miner, 0, id, 0, Action.TRANSPORTER_TAKE);
            else
                HW2Logger.getInstance().Log(0, s_smelter, id, 0, Action.TRANSPORTER_TAKE);

            Loaded();

            if (s_miner != 0)
            {
                if (t_smelter != 0)
                    HW2Logger.getInstance().Log(s_miner, t_smelter, id, 0, Action.TRANSPORTER_GO);
                else
                    HW2Logger.getInstance().Log(s_miner, 0, id, t_constructor, Action.TRANSPORTER_GO);     
            }
            else
                HW2Logger.getInstance().Log(0, s_smelter, id, t_constructor, Action.TRANSPORTER_GO);

            Sleep();

            if (t_smelter != 0)
                HW2Logger.getInstance().Log(0, t_smelter, id, 0, Action.TRANSPORTER_ARRIVE);
            else
                HW2Logger.getInstance().Log(0, 0, id, t_constructor, Action.TRANSPORTER_ARRIVE);

            WaitUnload();

            if (t_smelter != 0)
                HW2Logger.getInstance().Log(0, t_smelter, id, 0, Action.TRANSPORTER_DROP);
            else
                HW2Logger.getInstance().Log(0, 0, id, t_constructor, Action.TRANSPORTER_DROP);

            Unloaded();

            if (s_miner != 0)
            {
                if (t_smelter != 0)
                    HW2Logger.getInstance().Log(s_miner, t_smelter, id, 0, Action.TRANSPORTER_GO);
                else
                    HW2Logger.getInstance().Log(s_miner, 0, id, t_constructor, Action.TRANSPORTER_GO);
            }
            else
                HW2Logger.getInstance().Log(0, s_smelter, id, t_constructor, Action.TRANSPORTER_GO);
        }
        HW2Logger.getInstance().Log(0, 0, id, 0, Action.TRANSPORTER_STOPPED);
        Stop();
    }

    private boolean SourceHasMaterial()
    {
        //lock.lock();

        if (s_miner != 0)
        {
            //lock.unlock();
            return ((Miner) source).GetOutStorage() > 0;
        }
        else
        {
            //lock.unlock();
            return ((Smelter) source).GetOutStorage() > 0;
        }
    }

    private boolean SourceIsActive()
    {
        //lock.lock();

        return source_acive;
    }

    private void WaitNextLoad()
    {
        //lock.lock();
        
        // try {
        //     while (!SourceHasMaterial())
        //     {
        //         System.out.println("Transpoerter, source has not material");
        //         source_produced.await();
        //         System.out.println("Source produced material");
        //     }
        // } catch (InterruptedException e) {
        //     e.printStackTrace();
        // }

        if (s_miner != 0)
            ((Miner) source).OreLoaded();
        else if (s_smelter != 0)
            ((Smelter) source).IngotLoaded();

        System.out.println("Transporter loaded");

        //lock.unlock();
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

    private void WaitUnload()
    {
        lock.lock();

        System.out.println("Transporter Arrived");

        if (t_smelter != 0)
            ((Smelter) target).OreArrived();
        else if (t_constructor != 0)
            ((Constructor) target).IngotArrived();

        lock.unlock();
    }

    private void Loaded()
    {
        // if (s_miner != 0)
        //     ((Miner) source).OreLoaded();
        // else if (s_smelter != 0)
        //     ((Smelter) source).IngotLoaded();
    }

    private void Unloaded()
    {
        // TODO
    }

    public int GetMinerID()
    {
        return s_miner;
    }

    public int GetSSmelterID()
    {
        return s_smelter;
    }

    public int GetTSmelterID()
    {
        return t_smelter;
    }

    public int GetConstructorID()
    {
        return t_constructor;
    }

    // Miner informs transporters when ore is ready by calling this func.
    public void OreReady()
    {
        // TODO
    }

    // Flag miner as stopped 
    public void MinerStopped()
    {
        //System.out.println("Transporter: Miner Stopped!");
        source_acive = false;
    }

    // Get act if waiting for an ingot
    public void IngotReady()
    {
        // TODO
    }

    // Quit after transport all out_storage
    public void SmelterStopped()
    {
        source_acive = false;
    }

    private void Stop()
    {
        lock.lock();

        if (t_smelter != 0)
            Agents.getInstance().GetSmelterAgent(t_smelter).TransporterQuit();
        else if (t_constructor != 0)
            Agents.getInstance().GetConstructorAgent(t_constructor).TransporterQuit();

        lock.unlock();
    }
}
