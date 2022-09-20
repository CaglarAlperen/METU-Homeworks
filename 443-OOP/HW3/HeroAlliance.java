import java.util.Collections;
import java.util.HashSet;
import java.util.Set;


public class HeroAlliance {
    private final String name;
    private final int tier;
    private final Set<String> alliances;


    public HeroAlliance(String name, int tier, String ...args) {
        this.name = name;
        this.tier = tier;
        this.alliances = new HashSet<>();
        Collections.addAll(this.alliances, args);
    }

    @Override
    public String toString() {
        return "HeroAlliance{" +
                "name='" + name + '\'' +
                ", tier=" + tier +
                ", alliances=" + alliances +
                '}';
    }

    public String getName() {
        return name;
    }

    public int getTier() {
        return tier;
    }

    public Set<String> getAlliances() {
        return alliances;
    }

    public static HeroAlliance Parse(String line)
    {
        String[] params = line.split(",");
        if (params.length == 4)
        {
            return new HeroAlliance(params[0], Integer.parseInt(params[1]), params[2], params[3]);
        }
        else
        return new HeroAlliance(params[0], Integer.parseInt(params[1]), params[2], params[3], params[4]);
    }

}
