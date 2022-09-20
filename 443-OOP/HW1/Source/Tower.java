import java.awt.*;
import java.util.List;

public abstract class Tower extends Entity{

    protected int range;
    protected int rateOfFire;
    protected int damage;
    protected int cost;
    protected int kills;
    protected int counter;

    protected Monster hit;
    
    /**
     * Update function called in every frame.
     */
    public abstract void step();
    
    protected void drawRange(Graphics g, int x, int y)
    {
        Graphics2D copy = (Graphics2D) g.create();

        Stroke dashed = new BasicStroke(2, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 0, new float[]{9}, 0);
        copy.setStroke(dashed);

        x -= range - 25;
        y -= range - 25;

        copy.setColor(new Color(200,100,100));
        copy.drawOval(x, y, range*2, range*2);

        copy.dispose();
    }

    /**
     * Returns center point of tower object.
     * @return center point vector
     */
    public Vector2D getCenter()
    {
        return position.add(new Vector2D(Commons.TowerSize/2, Commons.TowerSize/2));
    }

    protected Monster getClosestMonster(List<Monster> monsters)
    {
        double minDistance = 2000;
        double distance;
        int index = 0;
        for (int i = 0; i < monsters.size(); i++)
        {
            distance = getCenter().distance(monsters.get(i).getCenter());
            if (distance < minDistance)
            {
                minDistance = distance;
                index = i;
            }
        }
        return monsters.get(index);
    }

    public void addKill()
    {
        kills++;
        if (kills == Commons.Grade1 || kills == Commons.Grade2 || kills == Commons.Grade3)
            Game.getInstance().updateTower(this, kills);
    }

    public int getKills()
    {
        return kills;
    }
}
