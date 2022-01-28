namespace VGL
{
    public class Window
    {
        public Window(uint width, uint height, string title)
        {
            var arroz = new VGLL.SimpleMath();
            Console.WriteLine($"{title} {arroz.add(5.5, 6.5)}");
        }
    }
}