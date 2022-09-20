public abstract class MonsterState implements IPaintable{
    
    public MonsterState(Monster monster)
    {
        this.monster = monster;
    }

    protected Monster monster;

    public abstract void update();
}
