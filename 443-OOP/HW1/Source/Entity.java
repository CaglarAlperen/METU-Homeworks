public abstract class Entity implements IPaintable{

    protected Vector2D position;

    /**
     * Returns position of entity
     * 
     * @return position of entity
     */
    public Vector2D getPosition()
    {
        return position;
    }

    /**
     * Update method called in every frame
     */
    public abstract void step();

}
