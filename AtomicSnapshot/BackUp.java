public class StampedSnap<T> {
	public long stamp;
	public T value;
	public T[] snap;
	public StampedSnap(T value) {
		stamp = 0;
		value = value;
		snap = null;
	}
	public StampedSnap(long label, T value, T[] snap) {
		label = label;
		value = value;
		snap = snap;
	}
}

public class WFSnapshot<T> implements Snapshot<T> {
	private StampedSnap<T>[] a_table; // array of atomic MRSW registers
	public WFSnapshot(int capacity, T init) {
		a_table = (StampedSnap<T>[]) new StampedSnap[capacity];
		for (int i = 0; i < a_table.length; i++) {
			a_table[i] = new StampedSnap<T>(init);
		}
	}
	private StampedSnap<T>[] collect() {
		StampedSnap<T>[] copy = (StampedSnap<T>[])
			new StampedSnap[a_table.length];
		for (int j = 0; j < a_table.length; j++)
			copy[j] = a_table[j];
		return copy;
	}
	public void update(T value) {
		int me = ThreadID.get();
		T[] snap = scan();
		StampedSnap<T> oldValue = a_table[me];
		StampedSnap<T> newValue =
			new StampedSnap<T>(oldValue.stamp+1, value, snap);
		a_table[me] = newValue;
	}
	public T[] scan() {
		StampedSnap<T>[] oldCopy;
		StampedSnap<T>[] newCopy;
		boolean[] moved = new boolean[a_table.length];
		oldCopy = collect();
collect: while (true) {
			 newCopy = collect();
			 for (int j = 0; j < a_table.length; j++) {
				 if (oldCopy[j].stamp != newCopy[j].stamp) {
					 if (moved[j]) {
						 return oldCopy[j].snap;;
					 } else {
						 moved[j] = true;
						 oldCopy = newCopy;
						 continue collect;
					 }
				 }
			 }
			 T[] result = (T[]) new Object[a_table.length];
			 for (int j = 0; j < a_table.length; j++)
				 result[j] = newCopy[j].value;
			 return result;
		 }
	}
}
