public class TowerIceFactory implements ITowerFactory {
    //TODO
    @Override
    public Tower createTower(Vector2D position) {
        return new TowerIce(position);
    }
}
