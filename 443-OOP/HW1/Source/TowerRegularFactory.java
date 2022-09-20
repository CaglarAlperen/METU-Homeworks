public class TowerRegularFactory implements ITowerFactory {

    //TODO
    @Override
    public Tower createTower(Vector2D position) {
        return new TowerRegular(position);
    }
}
