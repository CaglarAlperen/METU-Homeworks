import java.awt.*;

public class TowerRegular extends Tower{

    /**
     * Class constructor.
     * @param position position tower will be instantiated.
     */
    public TowerRegular(Vector2D position)
    {
        this.range = 150;
        this.rateOfFire = 20;
        this.damage = 20;
        this.cost = 20;
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
                hit = target;
            }
            else 
            {
                hit = null;
            }
        }
        else 
        {
            hit = null;
        }   
    }

    /**
     * Paints necessary drawings of tower object.
     */
    @Override
    public void paint(Graphics g) {
        g.setColor(Color.YELLOW);
        g.fillOval(position.getIntX(), position.getIntY(), Commons.TowerSize, Commons.TowerSize);
        drawRange(g, position.getIntX(), position.getIntY());

        if (hit != null)
        {
            g.setColor(Color.RED);
            g.drawChars(String.valueOf(hit.getHealth()).toCharArray(), 0, String.valueOf(hit.getHealth()).length(), hit.getPosition().getIntX()+10, hit.getPosition().getIntY()+22);
        }
    }
}
