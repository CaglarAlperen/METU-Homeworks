public class MonsterCircularStrategy implements IMonsterStrategy {
    
    /**
     * Updates direction to create circular motion around the tower grid.
     */
    @Override
    public Vector2D updateDirection(Vector2D position, Vector2D direction) 
    {
        if (direction.getIntX() == 0 && direction.getIntY() == -1) // Going Up
        {
            if (position.getIntY() <= 30)
            {
                return new Vector2D(1.0, 0.0);
            }
            else return direction;
        }
        else if (direction.getIntX() == 1 && direction.getIntY() == 0) // Going Right
        {
            if (position.getIntX() >= 330)
            {
                return new Vector2D(0.0, 1.0);
            }
            else return direction;
        }
        else if (direction.getIntX() == 0 && direction.getIntY() == 1) // Going Down
        {
            if (position.getIntY() >= 330)
            {
                return new Vector2D(-1.0, 0.0);
            }
            else return direction;
        }
        return direction;
    }
}
