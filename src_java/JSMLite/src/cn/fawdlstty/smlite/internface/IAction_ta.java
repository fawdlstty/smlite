package cn.fawdlstty.smlite.internface;

public interface IAction_ta<TTrigger extends Enum> {
    public void call (TTrigger trigger, Object[] args) throws Exception;
}
