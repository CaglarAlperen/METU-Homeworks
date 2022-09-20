import java.awt.*;

public class TowerIce extends Tower{
    
    /**
     * Class constructor.
     * @param position position tower will be instantiated.
     */
    public TowerIce(Vector2D position)
    {
        this.range = 100;
        this.rateOfFire = 20;
        this.damage = 10;
        this.cost = 15;
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
                target.setState(new MonsterIceState(target));
            }
        }  
    }

    /**
     * Paints necessary drawings of tower object.
     */
    @Override
    public void paint(Graphics g) {
        g.setColor(Color.BLUE);
        g.fillOval(position.getIntX(), position.getIntY(), Commons.TowerSize, Commons.TowerSize);
        drawRange(g, position.getIntX(), position.getIntY());
    }
}
