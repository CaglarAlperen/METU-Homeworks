import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Database {
    private static final Database _inst = new Database();
    public static Database getInstance() { return _inst; }

    private List<Hero> heroes;
    private List<HeroAlliance> heroAlliances;
    private List<Alliance> alliances;
    private List<Player> players;

    public Database() {
        //TODO optional
        heroes = new ArrayList<Hero>();
        heroAlliances = new ArrayList<HeroAlliance>();
        alliances = new ArrayList<Alliance>();
        players = new ArrayList<Player>();
    }

    //Parse the CSV files and fill the four lists given above.
    public void parseFiles(String playerCSVFile) throws IOException {
        Stream<String> lines;
        // Parse alliances.csv
        lines = Files.lines(Path.of("alliances.csv"));
        lines.forEach(s -> alliances.add(Alliance.Parse(s)));
        // Parse heroalliances.csv
        lines = Files.lines(Path.of("heroalliances.csv"));
        lines.forEach(s -> heroAlliances.add(HeroAlliance.Parse(s)));
        // Parse herostats.csv
        lines = Files.lines(Path.of("herostats.csv"));
        lines.forEach(s -> heroes.add(Hero.Parse(s)));
        // Parse playerCSVFile
        lines = Files.lines(Path.of(playerCSVFile));
        lines.forEach(s -> players.add(Player.Parse(s)));
    }

    //Gets the heroes belonging to a particular alliance and sorts them according to their DPS. It should only return
    //count number of heroes. Heroes should be distinct in a sense that, different levels of the same hero should not be
    //in the result.
    //15pts
    public List<Hero> getHeroesOfParticularAlliance(String alliance, int count) {
        String[] names = heroAlliances.stream().filter(h -> h.getAlliances().contains(alliance)).map(h -> h.getName()).toArray(String[]::new);
        Set<String> set = new HashSet<String>();
        List<Hero> result = heroes.stream().filter(h -> Arrays.asList(names).contains(h.getName())).sorted((h1,h2) -> h1.getDPS()>h2.getDPS()? -1 : 1).filter(h -> set.add(h.getName())).limit(count).collect(Collectors.toList());
        return result;
    }

    //Returns a map of HeroAlliances based on tier where the alliance required count and alliance level counts match.
    //15pts
    public Map<Integer, List<HeroAlliance>> getHeroAllianceMatchingTier(int allianceRequiredCount, int allianceLevelCount) {
        List<String> allianceList = alliances.stream().filter(a -> a.getLevelCount() == allianceLevelCount && a.getRequiredCount() == allianceRequiredCount).map(a -> a.getName()).collect(Collectors.toList());
        Map<Integer, List<HeroAlliance>> result = new HashMap<Integer, List<HeroAlliance>>();
        List<HeroAlliance> has = heroAlliances.stream().filter(h -> h.getAlliances().stream().filter(a -> allianceList.contains(a)).count() > 0).collect(Collectors.toList());
        System.out.println(has.size());
        has.stream().forEach(ha -> result.computeIfAbsent(ha.getTier(), k -> new ArrayList<>()).add(ha));
        return result;
    }

    //Return the heroes of each player that have bigger than the mana, health and move speed given as arguments.
    //10pts
    public List<List<Hero>> getPlayerHeros(int mana, int health, int moveSpeed) {
        List<List<Hero>> result = new ArrayList<List<Hero>>();
        players.stream().forEach(p -> result.add(p.getHeroes().stream().filter(h -> h.getMana() > mana && h.getHealth() > health && h.getMoveSpeed() > moveSpeed).collect(Collectors.toList())));
        return result;
    }

    //Calculate and print the average maximum damage of players whose heroes has minimum damage is bigger than the given first argument.
    //10 pts
    public void printAverageMaxDamage(int minDamage) {
        players.stream().forEach(p -> System.out.println(p.getHeroes().stream().filter(d -> d.getDamageMin() > minDamage).map(d -> d.getDamageMax()).mapToInt(Integer::intValue).average().getAsDouble()));
    }

    //In this function, print each player and its heroes. However, you should only print heroes belonging to
    // any of the particular alliances and whose attack speed is smaller than or equal to the value given.
    //30pts
    public void printAlliances(String[] alliances, double attackSpeed) {
        List<String> names = heroAlliances.stream().filter(ha -> Arrays.asList(alliances).stream().filter(a -> ha.getAlliances().contains(a)).count() > 0).map(ha -> ha.getName()).collect(Collectors.toList());
        List<Player> nonEmpty = players.stream().filter(p -> p.getHeroes().stream().filter(h -> names.contains(h.getName())).filter(h -> h.getAttackSpeed() <= attackSpeed).count() > 0).collect(Collectors.toList());
        nonEmpty.stream().forEach(p -> System.out.println(p.getName()+"\n"+p.getHeroes().stream().filter(h -> names.contains(h.getName())).filter(h -> h.getAttackSpeed() <= attackSpeed).map(h -> String.format("Name: %s Level: %s\n", h.getName(), h.getLevel())).reduce("", String::concat)));
    }

    public Hero getHero(String desc)
    {
        String[] fields = desc.split("\\|");
        return heroes.stream().filter(h -> h.getName().equals(fields[0]) && h.getLevel() == Integer.parseInt(fields[1])).findFirst().get();
    }
}
