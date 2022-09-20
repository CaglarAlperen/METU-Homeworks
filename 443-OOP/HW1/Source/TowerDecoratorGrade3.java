import java.awt.Color;
import java.awt.Graphics;

public class TowerDecoratorGrade3 extends TowerDecorator{

    public TowerDecoratorGrade3(Tower tower)
    {
        this.tower = tower;
        this.position = tower.getPosition();
    }

    /**
     * Paints decoration over the tower.
     */
    @Override
    public void paint(Graphics g) {
        tower.paint(g);

        Vector2D dotPosition = tower.getCenter().add(new Vector2D(Commons.TowerSize/3 - Commons.DecoratorDotSize/2, 0));
        g.setColor(Color.RED);
        g.fillOval(dotPosition.getIntX(), dotPosition.getIntY(), Commons.DecoratorDotSize, Commons.DecoratorDotSize);
        
    }

    /**
     * Calls decorated tower's step function.
     */
    @Override
    public void step() {
        tower.step();
        
    }
    //TODO
}
