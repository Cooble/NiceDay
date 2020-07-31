using System;
using System.Runtime.CompilerServices;
using ND;
public class TestingLayer:Layer
{
    public override void OnAttach() 
    {
        Log.ND_TRACE("TestingLayer attached");
        Log.ND_PROFILE_BEGIN_SESSION("startsession","startSes.json");
    }
    public override void OnDetach()
    {
        Log.ND_TRACE("TestingLayer detached");
        Log.ND_PROFILE_END_SESSION();

    }
}
