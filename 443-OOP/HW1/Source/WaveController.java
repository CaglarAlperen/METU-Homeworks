public class WaveController {
    
    private static final WaveController _inst = new WaveController();
    public static WaveController getInstance(){ return _inst; }

    /**
     * Class constructor instantiates wave count by 1.
     */
    public WaveController()
    {
        this.wave = 1;
    }

    private int wave;

    /**
     * Creates monsters according to wave number.
     */
    public void createMonsters()
    {
        for (int i = 0; i < wave; i++)
        {
            Monster monster = new Monster(100 /*+ wave * 20*/, 1.0, 10);
            Game.getInstance().addMonster(monster);
        }
        Display.getInstance().getInfoPanel().setWave(wave);
        wave++;
    }

    public int getWave()
    {
        return wave;
    }
}
