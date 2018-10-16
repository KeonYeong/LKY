package schedule;

import java.rmi.*;
import java.rmi.server.UnicastRemoteObject;
import java.util.Vector;
import java.util.HashMap;
import java.util.GregorianCalendar;

public class CalendarServant extends UnicastRemoteObject implements CalendarService{
	private HashMap<Integer, Triplet> calendar;
	private int ids = 0;

	public CalendarServant() throws RemoteException{
		calendar = new HashMap<Integer, Triplet>();
	}

	public int addSchedule(GregorianCalendar from, GregorianCalendar to, String desc) throws RemoteException{
		for(Triplet t : calendar.values()){
			if((t.getFrom().after(from) && t.getTo().before(from)) || (t.getFrom().after(to) && t.getTo().before(to))) return -1;
		}
		Triplet schedule = new Triplet(from, to, desc);
		calendar.put(++ids, schedule);
		return ids;
	}

	public void deleteSchedule(int id) throws RemoteException {
		if(calendar.isEmpty()) return;
		calendar.remove(id);
	}

	public Vector<Triplet> retrieveSchedule(GregorianCalendar from, GregorianCalendar to){
		Vector<Triplet> temp = new Vector<Triplet>();
		for(Triplet t : calendar.values()){
			if(t.getFrom().after(from) && t.getTo().before(to)) temp.add(t);
		}
		return temp;
	}
}
