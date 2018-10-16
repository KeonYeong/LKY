package client;

import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.Registry;
import java.rmi.registry.LocateRegistry;
import java.util.GregorianCalendar;
import java.util.Calendar;
import java.util.Scanner;
import java.util.Vector;

import schedule.CalendarService;
import schedule.Triplet;

public class Client{
	public Client() {}

	public static void main(String[] args){
		Scanner scan = new Scanner(System.in);
		CalendarService aCalendarService = null;
		try{
			Registry registry = LocateRegistry.getRegistry();
			aCalendarService = (CalendarService)registry.lookup("CalendarService");
			int cmd, year, month, day, schId, tmp;
			GregorianCalendar fromDate = null;
			GregorianCalendar toDate = null;
			String schInfo;
			while(true){
				System.out.println("Calendar Service :: 1. Add, 2. Delete, 3. Retrieve, 4. Quit");
				cmd = Integer.parseInt(scan.nextLine());
				switch(cmd){
					case 1:
						System.out.println("start from ::");
						System.out.print("year: ");
						year = Integer.parseInt(scan.nextLine());
						System.out.print("month: ");
						month = Integer.parseInt(scan.nextLine());
						System.out.print("day: ");
						day = Integer.parseInt(scan.nextLine());
						fromDate = new GregorianCalendar(year, month - 1, day);

						System.out.println("Until ::");
						System.out.print("year: ");
						year = Integer.parseInt(scan.nextLine());
						System.out.print("month: ");
						month = Integer.parseInt(scan.nextLine());
						System.out.print("day: ");
						day = Integer.parseInt(scan.nextLine());
						toDate = new GregorianCalendar(year, month - 1, day);

						System.out.print("Detail: ");
						schInfo = scan.nextLine();

						schId = aCalendarService.addSchedule(fromDate, toDate, schInfo);
						if(schId == -1) System.out.println("Failed");
						else System.out.println("Success, ID: " + schId);
						break;
					case 2:
						System.out.print("Which ID to Delete? ");
						tmp = Integer.parseInt(scan.nextLine());
						aCalendarService.deleteSchedule(tmp);
						System.out.println("Delete Complete");
						break;
					case 3:
						System.out.println("start from ::");
						System.out.print("year: ");
						year = Integer.parseInt(scan.nextLine());
						System.out.print("month: ");
						month = Integer.parseInt(scan.nextLine());
						System.out.print("day: ");
						day = Integer.parseInt(scan.nextLine());
						fromDate = new GregorianCalendar(year, month - 1, day);

						System.out.println("Until ::");
						System.out.print("year: ");
						year = Integer.parseInt(scan.nextLine());
						System.out.print("month: ");
						month = Integer.parseInt(scan.nextLine());
						System.out.print("day: ");
						day = Integer.parseInt(scan.nextLine());
						toDate = new GregorianCalendar(year, month - 1, day);

						Vector<Triplet> schedules = aCalendarService.retrieveSchedule(fromDate, toDate);
						if(schedules.size() == 0) {
							System.out.println("No Schedule available");
							break;
						}
						System.out.println("Schedules are ::");
						for(Triplet t : schedules){
							fromDate = t.getFrom();
							toDate = t.getTo();
							year = fromDate.get(Calendar.YEAR);
							month = fromDate.get(Calendar.MONTH) + 1;
							day = fromDate.get(Calendar.DATE);
							System.out.print(year + " / " + month + " / " + day + " ~ ");
							year = toDate.get(Calendar.YEAR);
							month = toDate.get(Calendar.MONTH) + 1;
							day = toDate.get(Calendar.DATE);
							System.out.println(year + " / " + month + " / " + day);
							schInfo = t.getDesc();
							System.out.println("Detail : " + schInfo);
						}
						break;
					case 4:
						scan.close();
						return;
				}
			}
		}catch(RemoteException e) {System.out.println(e.getMessage());
		}catch(Exception e) {System.out.println("Client: " + e.getMessage());}
	}
}
