public class TowerPoisonFactory implements ITowerFactory {

    //TODO
    @Override
    public Tower createTower(Vector2D position) {
        return new TowerPoison(position);
    }
}
