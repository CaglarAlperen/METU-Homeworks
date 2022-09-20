import java.util.ArrayList;

public class Agents {
    private static final Agents _inst = new Agents();
    public static Agents getInstance() {
        return _inst;
    }

    private ArrayList<Miner> miners = new ArrayList<Miner>();
    private ArrayList<Smelter> smelters = new ArrayList<Smelter>();
    private ArrayList<Constructor> constructors = new ArrayList<Constructor>();
    private ArrayList<Transporter> transporters = new ArrayList<Transporter>();

    public void AddMinerAgent(Miner miner)
    {
        miners.add(miner);
    }

    public void AddSmelterAgent(Smelter smelter)
    {
        smelters.add(smelter);
    }

    public void AddConstructorAgent(Constructor constructor)
    {
        constructors.add(constructor);
    }

    public void AddTransporterAgent(Transporter transporter)
    {
        transporters.add(transporter);
    }

    public Miner GetMinerAgent(int id)
    {
        return miners.get(id-1);
    }

    public Smelter GetSmelterAgent(int id)
    {
        return smelters.get(id-1);
    }

    public Constructor GetConstructorAgent(int id)
    {
        return constructors.get(id-1);
    }

    public Transporter GetTransporterAgent(int id)
    {
        return transporters.get(id-1);
    }

    public ArrayList<Miner> GetMiners()
    {
        return miners;
    }

    public ArrayList<Smelter> GetSmelters()
    {
        return smelters;
    }

    public ArrayList<Constructor> GetConstructors()
    {
        return constructors;
    }

    public ArrayList<Transporter> GetTransporters()
    {
        return transporters;
    }
}
