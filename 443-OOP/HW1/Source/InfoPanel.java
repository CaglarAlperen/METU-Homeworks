import javax.swing.*;
import java.awt.*;

public class InfoPanel extends JPanel {

    private JLabel currentGoldLabel;
    private JLabel currentLivesLabel;
    private JLabel currentKillsLabel;
    private JLabel currentWaveLabel;
    public InfoPanel() {


        this.setBackground(Color.WHITE);
        this.setPreferredSize(new Dimension(300, 400));
        this.setMinimumSize(new Dimension(300, 400));
        this.setSize(300, 400);
        this.setLayout(new GridLayout(4, 2));

        currentGoldLabel = new JLabel(String.valueOf(Game.getInstance().getGold()));
        currentLivesLabel = new JLabel(String.valueOf(Game.getInstance().getLives()));
        currentKillsLabel = new JLabel(String.valueOf(Game.getInstance().getKills()));
        currentWaveLabel = new JLabel(String.valueOf(WaveController.getInstance().getWave()));

        JLabel temp = new JLabel("Gold:");
        temp.setHorizontalAlignment(SwingConstants.RIGHT);
        temp.setFont(new Font("Serif", Font.BOLD, 20));
        temp.setForeground(Color.ORANGE);
        this.add(temp);this.add(currentGoldLabel);
        temp = new JLabel("Lives:");
        temp.setHorizontalAlignment(SwingConstants.RIGHT);
        temp.setFont(new Font("Serif", Font.BOLD, 20));
        temp.setForeground(Color.GREEN);
        this.add(temp);this.add(currentLivesLabel);
        temp = new JLabel("Kills:");
        temp.setHorizontalAlignment(SwingConstants.RIGHT);
        temp.setFont(new Font("Serif", Font.BOLD, 20));
        temp.setForeground(Color.RED);
        this.add(temp);this.add(currentKillsLabel);
        temp = new JLabel("Wave:");
        temp.setHorizontalAlignment(SwingConstants.RIGHT);
        temp.setFont(new Font("Serif", Font.BOLD, 20));
        this.add(temp);this.add(currentWaveLabel);


        currentGoldLabel.setHorizontalAlignment(SwingConstants.LEFT);
        currentLivesLabel.setHorizontalAlignment(SwingConstants.LEFT);
        currentKillsLabel.setHorizontalAlignment(SwingConstants.LEFT);
        currentWaveLabel.setHorizontalAlignment(SwingConstants.LEFT);

        currentGoldLabel.setFont(new Font("Serif", Font.BOLD, 20));
        currentLivesLabel.setFont(new Font("Serif", Font.BOLD, 20));
        currentKillsLabel.setFont(new Font("Serif", Font.BOLD, 20));
        currentWaveLabel.setFont(new Font("Serif", Font.BOLD, 20));

    }

    public void setGold(int gold)
    {
        currentGoldLabel.setText(String.valueOf(gold));
    }

    public void setLives(int lives)
    {
        currentLivesLabel.setText(String.valueOf(lives));
    }

    public void setKills(int kills)
    {
        currentKillsLabel.setText(String.valueOf(kills));
    }

    public void setWave(int wave)
    {
        currentWaveLabel.setText(String.valueOf(wave));
    }
}
