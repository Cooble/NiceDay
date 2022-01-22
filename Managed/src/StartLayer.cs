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

    private int e = 1;
    public override void OnUpdate()
    {

        base.OnUpdate();
       /* if((e++&255)==0)
            Log.ND_TRACE("updating c# koroshite kureor hfj");*/
    }

    public override void OnDetach()
    {
        Log.ND_TRACE("TestingLayer detached");
        Log.ND_PROFILE_END_SESSION();

    }
}
