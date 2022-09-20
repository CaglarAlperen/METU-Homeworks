import java.awt.*;

public class MonsterIceState extends MonsterState{

    private double icedSpeed;
    private int duration;
    private int counter;

    /**
     * Class constructor.
     * @param monster
     */
    public MonsterIceState(Monster monster)
    {
        super(monster);

        icedSpeed = 0.2;
        duration = 5;
        counter = 0;
    }

    /**
     * Update function called in every frame.
     */
    @Override
    public void update() {
        if (counter == 0)
            monster.setSpeed(icedSpeed);
        else if (counter == duration)
        {
            monster.setSpeed(1);
            monster.setState(null);
        }
        counter++;
    }

    /**
     * Paints necessary drawings of state.
     */
    @Override
    public void paint(Graphics g) {
        g.setColor(Color.BLUE);
        g.drawRect(monster.getPosition().getIntX(), monster.getPosition().getIntY(), Commons.MonsterSize, Commons.MonsterSize);
    }
}
