package cn.fawdlstty.smlite;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

enum MyState { Rest, Ready, Reading, Writing };
enum MyTrigger { Run, Close, Read, FinishRead, Write, FinishWrite };

class Assert {
    static void IsTrue (boolean b) throws Exception { if (!b) throw new Exception ("IsTrue"); }
    static void IsFalse (boolean b) throws Exception { if (b) throw new Exception ("IsFalse"); }
    static void AreEqual (Object a, Object b) throws Exception { if (!a.equals(b)) throw new Exception ("AreEqual"); }
}

public class Main {
    static void TestMethod1 () throws Exception {
        AtomicInteger n = new AtomicInteger(0);
        AtomicBoolean entry_one = new AtomicBoolean(true);
        SMLiteBuilder<MyState, MyTrigger> _smb = new SMLiteBuilder ();
        _smb.Configure (MyState.Rest)
                .OnEntry (() -> { Assert.IsFalse (entry_one.get()); entry_one.set(true); n.addAndGet(1); })
				.OnLeave (() -> { Assert.IsTrue (entry_one.get()); entry_one.set(false); n.addAndGet(10); })
				.WhenChangeTo (MyTrigger.Run, MyState.Ready)
                .WhenIgnore (MyTrigger.Close);
        _smb.Configure (MyState.Ready)
                .OnEntry (() -> { Assert.IsFalse (entry_one.get()); entry_one.set(true); n.addAndGet(100); })
				.OnLeave (() -> { Assert.IsTrue (entry_one.get()); entry_one.set(false); n.addAndGet(1000); })
				.WhenChangeTo (MyTrigger.Read, MyState.Reading)
                .WhenChangeTo (MyTrigger.Write, MyState.Writing)
                .WhenChangeTo (MyTrigger.Close, MyState.Rest);
        _smb.Configure (MyState.Reading)
                .OnEntry (() -> { Assert.IsFalse (entry_one.get()); entry_one.set(true); n.addAndGet(10000); })
				.OnLeave (() -> { Assert.IsTrue (entry_one.get()); entry_one.set(false); n.addAndGet(100000); })
				.WhenChangeTo (MyTrigger.FinishRead, MyState.Ready)
                .WhenChangeTo (MyTrigger.Close, MyState.Rest);
        _smb.Configure (MyState.Writing)
                .OnEntry (() -> { Assert.IsFalse (entry_one.get()); entry_one.set(true); n.addAndGet(1000000); })
				.OnLeave (() -> { Assert.IsTrue (entry_one.get()); entry_one.set(false); n.addAndGet(10000000); })
				.WhenChangeTo (MyTrigger.FinishWrite, MyState.Ready)
                .WhenChangeTo (MyTrigger.Close, MyState.Rest);

        SMLite<MyState, MyTrigger> _sm = _smb.Build (MyState.Rest);
        Assert.AreEqual (_sm.GetState (), MyState.Rest);
        Assert.AreEqual (n.get(), 0);
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Run));
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Close));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.Read));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishRead));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.Write));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishWrite));

        _sm.Triggering (MyTrigger.Close);
        Assert.AreEqual (_sm.GetState(), MyState.Rest);
        Assert.AreEqual (n.get(), 0);

        _sm.Triggering (MyTrigger.Close);
        Assert.AreEqual (_sm.GetState(), MyState.Rest);
        Assert.AreEqual (n.get(), 0);

        _sm.Triggering (MyTrigger.Close);
        Assert.AreEqual (_sm.GetState(), MyState.Rest);
        Assert.AreEqual (n.get(), 0);

        _sm.Triggering (MyTrigger.Run);
        Assert.AreEqual (_sm.GetState(), MyState.Ready);
        Assert.AreEqual (n.get(), 110);
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.Run));
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Close));
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Read));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishRead));
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Write));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishWrite));

        _sm.Triggering (MyTrigger.Close);
        Assert.AreEqual (_sm.GetState(), MyState.Rest);
        Assert.AreEqual (n.get(), 1111);
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Run));
        Assert.IsTrue (_sm.AllowTriggering (MyTrigger.Close));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.Read));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishRead));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.Write));
        Assert.IsFalse (_sm.AllowTriggering (MyTrigger.FinishWrite));

        _sm.Triggering (MyTrigger.Run);
        Assert.AreEqual (_sm.GetState(), MyState.Ready);
        Assert.AreEqual (n.get(), 1221);

        _sm.Triggering (MyTrigger.Read);
        Assert.AreEqual (_sm.GetState(), MyState.Reading);
        Assert.AreEqual (n.get(), 12221);

        _sm.Triggering (MyTrigger.FinishRead);
        Assert.AreEqual (_sm.GetState(), MyState.Ready);
        Assert.AreEqual (n.get(), 112321);

        _sm.Triggering (MyTrigger.Write);
        Assert.AreEqual (_sm.GetState(), MyState.Writing);
        Assert.AreEqual (n.get(), 1113321);

        _sm.Triggering (MyTrigger.FinishWrite);
        Assert.AreEqual (_sm.GetState(), MyState.Ready);
        Assert.AreEqual (n.get(), 11113421);

        _sm.SetState(MyState.Reading);
        _sm.SetState(MyState.Writing);
        _sm.SetState(MyState.Rest);
        Assert.AreEqual (n.get(), 11113421);
    }

    public static void main(String[] args) throws Exception {
        TestMethod1 ();
        System.out.println ("Test Success");
    }
}
