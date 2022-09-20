public interface ITowerFactory {
    /**
     * Factory method creates tower.
     * @param position position tower will be instantiated.
     * @return created Tower
     */
    Tower createTower(Vector2D position);
}
