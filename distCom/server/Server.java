package server;

import java.rmi.*;
import java.rmi.registry.Registry;
import java.rmi.registry.LocateRegistry;

import schedule.CalendarService;
import schedule.CalendarServant;

public class Server{
	public static void main(String args[]){
		try{
			CalendarService aCalendarService = new CalendarServant();
			Registry registry = LocateRegistry.createRegistry(1099);
			registry.rebind("CalendarService", aCalendarService);
			System.out.println("Server Ready");
		}catch(Exception e){
			System.out.println("Server Main " + e.getMessage());
		}
	}
}
