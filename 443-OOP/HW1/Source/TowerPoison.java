import java.awt.*;

public class TowerPoison extends Tower{
   
    /**
     * Class constructor.
     * @param position position tower will be instantiated.
     */
    public TowerPoison(Vector2D position)
    {
        this.range = 75;
        this.rateOfFire = 10;
        this.damage = 5;
        this.cost = 25;
        this.kills = 0;
        this.counter = 0;
        this.position = position.add(new Vector2D(5,5));
    }

    /**
     * Update function called in every frame.
     */
    @Override
    public void step() {
        if (counter++ % rateOfFire == 0)
        {
            Monster target = getClosestMonster(Game.getInstance().getMonsters());
            if (getCenter().distance(target.getCenter()) <= range)
            {
                target.Damage(damage, this);
                target.setState(new MonsterPoisonState(target, this));
            }
        }
    }

    /**
     * Paints necessary drawings of tower object.
     */
    @Override
    public void paint(Graphics g) {
        g.setColor(Color.GREEN);
        g.fillOval(position.getIntX(), position.getIntY(), Commons.TowerSize, Commons.TowerSize);
        drawRange(g, position.getIntX(), position.getIntY());
    }
}
