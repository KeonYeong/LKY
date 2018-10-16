package schedule;

import java.io.Serializable;
import java.util.GregorianCalendar;

public class Triplet implements Serializable {
	private GregorianCalendar from;
	private GregorianCalendar to;
	private String desc;

	public Triplet(GregorianCalendar from, GregorianCalendar to, String desc){
		this.from = from;
		this.to = to;
		this.desc = desc;
	}

	public GregorianCalendar getFrom(){
		return from;
	}

	public void setFrom(GregorianCalendar from){
		this.from = from;
	}

	public GregorianCalendar getTo(){
		return to;
	}

	public void setTo(GregorianCalendar to){
		this.to = to;
	}

	public String getDesc(){
		return desc;
	}

	public void setDesc(String desc){
		this.desc = desc;
	}
}
