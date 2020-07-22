using System;
public class Dog
{
    static public void Statico()
    {
        Console.WriteLine("Calling statico and Simpl!");
        Log.ND_INFO("Hello there supdawg this is dll ant sooo sweet man and so fast its really great!" + 10);

    }
    public void Bark()
    {
        Console.WriteLine("bark!");
    }
    public void Bark(int times)
    {
        
        for (var i = 0; i < times; ++i )
            Console.WriteLine("bark!");

    }
}