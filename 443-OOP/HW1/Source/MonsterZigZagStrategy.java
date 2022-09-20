public class MonsterZigZagStrategy implements IMonsterStrategy {
    
    /**
     * Updates direction to create zig zag motion around the tower grid.
     */
    @Override
    public Vector2D updateDirection(Vector2D position, Vector2D direction) 
    {
        if (position.getIntX() == 0 || position.getIntX() == 360)
            return new Vector2D(direction.getIntX()*-1, direction.getIntY());
        if (position.getIntY() == 0 || position.getIntY() == 360)
            return new Vector2D(direction.getIntX(), direction.getIntY()*-1);
        if (position.getIntY() > 60 && position.getIntY() < 300)
        {
            if (position.getIntX() == 60 || position.getIntX() == 300)
                return new Vector2D(direction.getIntX()*-1, direction.getIntY());
        }
        if (position.getIntX() > 60 && position.getIntX() < 300)
        {
            if (position.getIntY() == 60 || position.getIntY() == 300)
                return new Vector2D(direction.getIntX(), direction.getIntY()*-1);
        }
        return direction;     
    }
}
