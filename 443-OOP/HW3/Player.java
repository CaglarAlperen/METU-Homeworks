import java.util.*;
import java.util.stream.Stream;

public class Player {
    private final String name;
    private final List<Hero> heroes;

    public Player(String name, List<Hero> heroes) {
        this.name = name;
        this.heroes = heroes;
    }

    public Player(String name) {
        this.name = name;
        this.heroes = new ArrayList<>();
    }

    public String getName() {
        return name;
    }

    public List<Hero> getHeroes() {
        return heroes;
    }

    @Override
    public String toString() {
        return "Player{" +
                "name='" + name + '\'' +
                ", heroes=" + heroes +
                '}';
    }

    public static Player Parse(String line)
    {
        String[] params = line.split(",");
        String playername = params[0];
        String[] desc = new String[params.length-1];
        System.arraycopy(params, 1, desc, 0, params.length-1);
        List<Hero> heroes = new ArrayList<Hero>();
        Stream.of(desc).forEach(h -> heroes.add(Database.getInstance().getHero(h)));
        return new Player(playername, heroes);
    }
}
