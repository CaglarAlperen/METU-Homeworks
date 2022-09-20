import java.io.IOException;

public class main {
    public static void main(String[] args) {
        try {
            Database.getInstance().parseFiles("players.csv");
        } catch (IOException e) {
            e.printStackTrace();
        }
        var result = Database.getInstance().getHeroesOfParticularAlliance("Mage", 3); 
        result.forEach(r -> System.out.println(r.toString()));
    }
}
