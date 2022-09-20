import java.awt.*;

public class MonsterPoisonState extends MonsterState{

    private int poisonDamage;
    private int duration;
    private int count;
    private Tower tower;

    /**
     * Class constructor.
     * @param monster
     */
    public MonsterPoisonState(Monster monster, Tower tower)
    {
        super(monster);

        this.tower = tower;
        poisonDamage = 5;
        duration = 3;
        count = 0;
    }

    /**
     * Update function called in every frame.
     */
    @Override
    public void update() {
        if (count == duration)
        {
            monster.setState(null);
        }
        else
        {
            monster.Damage(poisonDamage, tower);
        }
        count++;
    }

    /**
     * Paints necessary drawings of state.
     */
    @Override
    public void paint(Graphics g) {
        g.setColor(Color.GREEN);
        g.drawRect(monster.getPosition().getIntX(), monster.getPosition().getIntY(), Commons.MonsterSize, Commons.MonsterSize);
    }
}
