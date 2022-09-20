public class Alliance {
    private final String name;
    private final int requiredCount;
    private final int levelCount;

    public Alliance(String name, int requiredCount, int levelCount) {
        this.name = name;
        this.requiredCount = requiredCount;
        this.levelCount = levelCount;
    }

    public String getName() {
        return name;
    }

    public int getRequiredCount() {
        return requiredCount;
    }

    public int getLevelCount() {
        return levelCount;
    }

    public static Alliance Parse(String line)
    {
        //System.out.println("Parse Called! " + line);
        String[] params = line.split(",",3);
        //System.out.println(params[0] + " " + params[1] + " " + params[2]);
        return new Alliance(params[0], Integer.parseInt(params[1]), Integer.parseInt(params[2]));
    }
}
