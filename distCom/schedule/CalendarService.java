package schedule;

import java.rmi.*;
import java.util.GregorianCalendar;
import java.util.Vector;

public interface CalendarService extends Remote{
	int addSchedule(GregorianCalendar from, GregorianCalendar to, String desc) throws RemoteException;
	void deleteSchedule(int id) throws RemoteException;
	Vector<Triplet> retrieveSchedule(GregorianCalendar from, GregorianCalendar to) throws RemoteException;
}
