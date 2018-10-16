package client;

import java.rmi.*;
import java.rmi.server.*;
import java.util.GregorianCalendar;

import schedule.CalendarService;

public class Client{
	public Client() {}

	public static void main(String[] args){
		CalendarService aCalendarService = null;
		try{
			aCalendarService = (CalendarService)Naming.lookup("rmi://127.0.0.1/CalendarService");
		}catch(RemoteException e) {System.out.println(e.getMessage());
		}catch(Exception e) {System.out.println("Client: " + e.getMessage());}
	}
}
