import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class Game 
{
    private static final Game _inst = new Game();
    public static Game getInstance() 
    {
        return _inst;
    }
   
    private static Timer loop;
    private int gold;
    private int lives;
    private int kills;
    private List<Monster> monsters;
    private List<Tower> towers;
    private Vector2D activeGridPos = null;

    private List<Monster> deadMonsters;

    /**
     * Class constructor
     */
    public Game() 
    {
        this.gold = 25;
        this.lives = 3;
        this.kills = 0;
        this.monsters = new ArrayList<Monster>();
        this.towers = new ArrayList<Tower>();
        this.deadMonsters = new ArrayList<Monster>();

        loop = new Timer(5, actionEvent -> 
        {
            Game.getInstance().step();
        });
    }

    /**
     * Returns current gold
     * @return current gold
     */
    public int getGold() { return this.gold; }
    /**
     * Returns life count
     * @return life count
     */
    public int getLives() { return this.lives; }
    /**
     * Returns kill count
     * @return kill count
     */
    public int getKills() { return this.kills; }
    /**
     * Returns alive monsters list
     * @return monster list
     */
    public List<Monster> getMonsters() { return this.monsters; }

    /**
     * Adds monster to the monsters list.
     * @param monster
     */
    public void addMonster(Monster monster)
    {
        monsters.add(monster);
    }

    /**
     * Removes monster from monsters list.
     * Adds gold reward of monster.
     * Updates info panel.
     * @param monster
     */
    public void killMonster(Monster monster)
    {
        deadMonsters.add(monster);
        addGold(monster.getReward());
        kills++;
        Display.getInstance().getInfoPanel().setKills(kills);
    }

    /**
     * Adds reward to gold variable.
     * Updates info panel.
     * @param reward reward amount of monster
     */
    public void addGold(int reward)
    {
        gold += reward;
        Display.getInstance().getInfoPanel().setGold(gold);
    }

    /**
     * Removes one life from player.
     * If player lost oll of lives game is over.
     */
    public void removeLife()
    {
        lives--;
        Display.getInstance().getInfoPanel().setLives(lives);
        if (lives <= 0)
            loop.stop();
    }

    /**
     * Sets active grid position which is clicked area of tower grid.
     * If position is outside of the area it removes current active position.
     * 
     * @param pos snapped position of mouse input according to tower grid.
     */
    public void setActiveGridPos(Vector2D pos)
    {
        if (pos.getIntX() != -1)
        {
            for (Tower tower : towers)
            {
                if (pos.equals(tower.getPosition().add(new Vector2D(-5,-5))))
                {
                    return;
                }
            }
            activeGridPos = pos;
        }
        else
            activeGridPos = null;
    }

    /**
     * Checks tower grid to place selected tower .
     * If position is available creates new tower according to parameter.
     * @param c user keyboard input which tells tower type.
     */
    public void checkForBuildTower(char c)
    {
        if (activeGridPos == null)
            return;

        if (c == 'r')
        {
            if (gold < Commons.TowerRegularCost)
                return;
            Tower tower = new TowerRegularFactory().createTower(activeGridPos);
            towers.add(tower);
            gold -= Commons.TowerRegularCost;
            activeGridPos = null;
        }
        else if (c == 'i')
        {
            if (gold < Commons.TowerIceCost)
                return;
            Tower tower = new TowerIceFactory().createTower(activeGridPos);
            towers.add(tower);
            gold -= Commons.TowerIceCost;
            activeGridPos = null;
        }
        else if (c == 'p')
        {
            if (gold < Commons.TowerPoisonCost)
                return;
            Tower tower = new TowerPoisonFactory().createTower(activeGridPos);
            towers.add(tower);
            gold -= Commons.TowerPoisonCost;
            activeGridPos = null;
        }
        Display.getInstance().getInfoPanel().setGold(gold);
    }

    /**
     * Updates tower decoration after reaching certain amount of kills.
     * @param tower tower will be decorated.
     * @param kills kill count of that tower.
     */
    public void updateTower(Tower tower, int kills)
    {
        for (int i = 0; i < towers.size(); i++)
        {
            if (towers.get(i).getPosition() == tower.getPosition())
            {
                if (kills == Commons.Grade1)
                    towers.set(i, new TowerDecoratorGrade1(towers.get(i)));
                else if (kills == Commons.Grade2)
                    towers.set(i, new TowerDecoratorGrade2(towers.get(i)));
                else if (kills == Commons.Grade3)
                    towers.set(i, new TowerDecoratorGrade3(towers.get(i)));
            }
        }
    }

    /**
     * Paints monsters, towers and active grid position every frame.
     */
    public void paint(Graphics g) 
    {
        for (Monster monster : monsters) 
        {
            monster.paint(g);
        }
        for (Tower tower : towers)
        {
            tower.paint(g);
        }

        if (activeGridPos != null)
        {
            g.setColor(Color.WHITE);
            g.fillRect(activeGridPos.getIntX(), activeGridPos.getIntY(), Commons.TowerZoneDivideLength, Commons.TowerZoneDivideLength);
        }
    }

    /**
     * Core game loop called in every frame which updates game.
     */
    public void step() 
    {
        if (monsters.size() == 0)
        {
            WaveController.getInstance().createMonsters();
        }

        for (Tower tower : towers)
        {
            tower.step();
        }

        for (Monster monster : monsters) 
        {
            monster.step();
        }

        for (Monster monster : deadMonsters)
        {
            monsters.remove(monster);
        }

        Display.getInstance().repaint();
    }

    /**
     * Starts game.
     */
    public static void startGame() 
    {
        Display.getInstance().setVisible(true);
        //Optional additions

        loop.start();
    }

    public static void main(String[] args) 
    {
        SwingUtilities.invokeLater(Game::startGame);
    }
}
