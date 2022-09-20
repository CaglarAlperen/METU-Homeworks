import java.util.ArrayList;
import java.util.Scanner;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Simulator {
    public static void main(String[] args) throws InterruptedException 
    {
        Scanner scanner = new Scanner(System.in);
        ExecutorService executor = Executors.newCachedThreadPool();
        ArrayList<Runnable> agents = new ArrayList<Runnable>();

        int id = 1;
        int num_of_miners = scanner.nextInt();

        // Initialize Logger
        HW2Logger.getInstance().Initialize();

        for (int i = 0; i < num_of_miners; i++)
        {
            int interval = scanner.nextInt();
            int capacity = scanner.nextInt();
            int type = scanner.nextInt();
            int retire = scanner.nextInt();

            Miner miner = new Miner(id++, interval, capacity, type, retire);
            Agents.getInstance().AddMinerAgent(miner);
            agents.add(miner);
            //executor.execute(miner);
        }

        int num_of_smelters = scanner.nextInt();
        id = 1;

        for (int i = 0; i < num_of_smelters; i++)
        {
            int interval = scanner.nextInt();
            int in_storage = scanner.nextInt();
            int out_storage = scanner.nextInt();
            int type = scanner.nextInt();

            Smelter smelter = new Smelter(id++, interval, in_storage, out_storage, type);
            Agents.getInstance().AddSmelterAgent(smelter);
            agents.add(smelter);
            //executor.execute(smelter);
        }

        int num_of_constructors = scanner.nextInt();
        id = 1;

        for (int i = 0; i < num_of_constructors; i++) 
        {
            int interval = scanner.nextInt();
            int capacity = scanner.nextInt();
            int type = scanner.nextInt();

            Constructor constructor = new Constructor(id++, interval, capacity, type);
            Agents.getInstance().AddConstructorAgent(constructor);
            agents.add(constructor);
            //executor.execute(constructor);
        }

        int num_of_transporters = scanner.nextInt();
        id = 1;

        for (int i = 0; i < num_of_transporters; i++)
        {
            int interval = scanner.nextInt();
            int s_miner = scanner.nextInt();
            int s_smelter = scanner.nextInt();
            int t_smelter = scanner.nextInt();
            int t_constructor = scanner.nextInt();

            Transporter transporter = new Transporter(id++, interval, s_miner, s_smelter, t_smelter, t_constructor);
            Agents.getInstance().AddTransporterAgent(transporter);
            agents.add(transporter);
            //executor.execute(transporter);
        }

        for (Runnable agent : agents) 
        {
            executor.execute(agent);
        }

        executor.shutdown();

        while (!executor.isTerminated()){}

        scanner.close();
    }
}
