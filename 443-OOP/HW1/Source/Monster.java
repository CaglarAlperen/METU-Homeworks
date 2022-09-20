import java.awt.*;
import java.util.Random;

public class Monster extends Entity {
    
    private int health;
    private double speed;
    private Vector2D direction;
    private int reward;
    private IMonsterStrategy strategy;
    private MonsterState state = null;

    /**
     * Constructor method.
     * Sets random strategy.
     * Sets random starting point in starting zone.
     * @param health 
     * @param speed
     * @param reward
     */
    public Monster(int health, double speed, int reward)
    {
        this.health = health;
        this.speed = speed;
        this.reward = reward;
        this.strategy = getRandomStrategy();
        this.direction = (strategy instanceof MonsterCircularStrategy) ? new Vector2D(0.0, -1.0) : new Vector2D(-1.0, -1.0);
        this.position = getRandomStartPoint();
    }

    /**
     * Update function called in every frame.
     */
    @Override
    public void step() 
    {
        position = position.add(direction.multiply(speed));
        direction = strategy.updateDirection(position, direction);

        if (state != null)
        {
            state.update();
        }

        if (enterStartZone())
        {
            Game.getInstance().removeLife();
            Game.getInstance().killMonster(this);
        }
    }

    /**
     * Paints necessary drawings of monster object.
     */
    @Override
    public void paint(Graphics g) 
    {
        g.setColor(Color.ORANGE);
        g.fillRect(position.getIntX(), position.getIntY(), Commons.MonsterSize, Commons.MonsterSize);
        g.setColor(Color.WHITE);
        g.drawChars(String.valueOf(health).toCharArray(), 0, String.valueOf(health).length(), position.getIntX()+10, position.getIntY()+22);

        if (state != null)
        {
            state.paint(g);
        }
    }

    private IMonsterStrategy getRandomStrategy()
    {
        Random rand = new Random();
        int result = rand.nextInt(2);
        if (result == 0)
            return new MonsterCircularStrategy();
        else
            return new MonsterZigZagStrategy();
    }

    private Vector2D getRandomStartPoint()
    {
        Random rand = new Random();
        int x = rand.nextInt(Commons.StartWidth - Commons.MonsterSize - 1) + Commons.StartX + 1;
        int y = rand.nextInt(Commons.StartHeight - Commons.MonsterSize) + Commons.StartY;
        return new Vector2D(x, y);
    }

    public void setSpeed(double speed)
    {
        this.speed = speed;
    }

    public void setState(MonsterState state)
    {
        this.state = state;
    }

    public Vector2D getCenter()
    {
        return position.add(new Vector2D(Commons.MonsterSize/2, Commons.MonsterSize/2));
    }

    public int getHealth()
    {
        return health;
    }

    public int getReward()
    {
        return reward;
    }

    /**
     * Decrease health of monster amoun of damage.
     * If health is less than damage kills monster.
     * @param damage damage will be dealt
     * @param tower tower damage came from
     */
    public void Damage(int damage, Tower tower)
    {
        health -= damage;
        if (health <= 0) 
        {
            Game.getInstance().killMonster(this);
            tower.addKill();
        }
    }

    private boolean enterStartZone()
    {
        if (position.getIntY() > Commons.StartY && position.getIntX() == Commons.StartX+Commons.StartWidth)
            return true;
        return false;
    }
}
