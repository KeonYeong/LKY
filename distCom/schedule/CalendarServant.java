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
			if((t.getFrom().before(from) && t.getTo().after(from)) || (t.getFrom().before(to) && t.getTo().after(to))) return -1;
			if(t.getFrom().equals(from) || t.getFrom().equals(to) || t.getTo().equals(from) || t.getTo().equals(to)) return -1;
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
			else if((t.getFrom().equals(from) && t.getTo().before(to)) || (t.getFrom().after(from) && t.getTo().equals(to)) || (t.getFrom().equals(from) && t.getTo().equals(to))) temp.add(t);
		}
		return temp;
	}
}
