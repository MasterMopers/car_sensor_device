import javax.swing.JFrame;
import java.awt.Canvas;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.image.BufferStrategy;

class Launcher {

	public static void main(String[] args) {
		Game game = new Game(1600, 900, "TheMansion");
		game.start();
	}
}

class Display {

	public JFrame frame;
	public Canvas canvas;
	public int width, height;

	public Display(int width, int height, String title) {
		this.width = width;
		this.height = height;

		frame = new JFrame();
		frame.setTitle(title);
		frame.setSize(width, height);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setResizable(false);
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);

		canvas = new Canvas();
		canvas.setPreferredSize(new Dimension(width, height));
		canvas.setMinimumSize(new Dimension(width, height));
		canvas.setMaximumSize(new Dimension(width, height));

		frame.add(canvas);
		frame.pack();
	}
}

class Game implements Runnable {

	public int width, height, fps = 60;
	private String title;
	public Display display;

	public Thread thread;
	public boolean running = false;
	private BufferStrategy bs;
	private Graphics g;

	public Game(int width, int height, String title) {
		this.width = width;
		this.height = height;
		this.title = title;
	}

	private void init() {
		display = new Display(width, height, title);
	}

	private void tick() {
    //update variables here
	}

	private void render() {
		bs = display.canvas.getBufferStrategy();
		if(bs == null) {
			display.canvas.createBufferStrategy(3);
			return;
		}

		g = bs.getDrawGraphics();
		g.clearRect(0, 0, width, height);

    //draw here
    
		bs.show();
		g.dispose();
	}

	public void run() {
		init();

		double tickDuration = 1000000000 / fps;
		double delta = 0;
		long now;
		long lastTime = System.nanoTime();

		while(running) {
			now = System.nanoTime();
			delta += (now - lastTime) / tickDuration;
			lastTime = now;

			if(delta >= 1) {
				tick();
				render();
				delta--;
			}
		}

		stop();
	}

	public synchronized void start() {
		if(running) return;
		running = true;
		thread = new Thread(this);
		thread.start();
	}

	public synchronized void stop() {
		if(!running) return;
		running = false;
		try {
			thread.join();
		} catch(InterruptedException e) {
			e.printStackTrace();
		}
	}
}