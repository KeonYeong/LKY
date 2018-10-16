package server;

import java.rmi.*;

import schedule.CalendarService;
import schedule.CalendarServant;

public class Server{
	public static void main(String args[]){
		try{
			CalendarService aCalendarService = new CalendarServant();
			Naming.rebind("CalendarService", aCalendarService);
			System.out.println("Server Ready");
		}catch(Exception e){
			System.out.println("Server Main " + e.getMessage());
		}
	}
}
