public interface IMonsterStrategy {
    /**
     * Updates direction of monster according to strategy.
     * @param position position of monster
     * @param direction current direction vector of monster.
     * @return new position.
     */
    Vector2D updateDirection(Vector2D position, Vector2D direction);
}
