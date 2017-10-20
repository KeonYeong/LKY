use Pokemon;
SELECT name from Trainer where hometown LIKE 'Blue City';
SELECT name from Trainer where hometown LIKE 'Rainbow City' or hometown LIKE 'Brown City';
SELECT name, hometown from Trainer where name LIKE 'a%' or name LIKE 'e%' OR name LIKE 'i%' OR name LIKE 'o%' OR name LIKE 'u%';
SELECT name from Pokemon where type LIKE 'Water';
SELECT DISTINCT type from Pokemon;
SELECT name from Pokemon ORDER BY name ASC;
SELECT name from Pokemon where name LIKE '%s';
SELECT name from Pokemon where name LIKE '%e%s';
SELECT name from Pokemon where name LIKE 'a%' OR name LIKE 'e%' OR name LIKE 'i%' OR name LIKE 'o%' OR name LIKE'u%';
SELECT type, count(*) from Pokemon GROUP BY type;
SELECT nickname from CatchedPokemon ORDER BY level DESC LIMIT 3;
SELECT avg(level) as Average from CatchedPokemon;
SELECT max(level) - min(level) AS Difference from CatchedPokemon;
SELECT count(name) from Pokemon where name LIKE 'b%' or name LIKE 'd%' or name LIKE 'e%';
SELECT count(type) from Pokemon where type NOT LIKE 'Fire' AND type NOT LIKE 'Grass' AND type NOT LIKE 'Water' AND type NOT LIKE 'Electric';
SELECT t.name, p.name as pokemon, c.nickname from Trainer as t, Pokemon as p, CatchedPokemon as c where c.nickname like '% %' and p.id = c.pid and t.id = c.owner_id;
SELECT t.name from CatchedPokemon as c, Trainer as t, Pokemon as p where c.pid = p.id and p.type like 'Psychic' and c.owner_id = t.id;
SELECT t.name, t.hometown from Trainer as t, CatchedPokemon as c where t.id = c.owner_id GROUP BY c.owner_id ORDER BY avg(c.level) DESC LIMIT 3;
SELECT t.name, count(*) from Trainer as t, CatchedPokemon as c where c.owner_id = t.id group by c.owner_id order by count(*) desc;
SELECT p.name, c.level from Gym as g, Pokemon as p, CatchedPokemon as c where g.city LIKE 'Sangnok City' and g.leader_id = c.owner_id and p.id = c.pid order by c.level asc;
select p.name, count(c.pid) as catched_number from Pokemon as p left join CatchedPokemon as c on p.id = c.pid group by p.id order by count(c.pid) desc;
SELECT p.name from Pokemon as p, Evolution as e where e.before_id in ( select e.after_id from Pokemon as p, Evolution as e where p.name like 'Charmander' and p.id = e.before_id) and e.after_id = p.id;
SELECT distinct name from Pokemon where id <= 30 and id in (select pid from CatchedPokemon) order by name asc;
SELECT t.name, p.type from CatchedPokemon as c JOIN Pokemon as p on p.id = c.pid JOIN Trainer as t on t.id = c.owner_id group by c.owner_id having count(distinct p.type) = 1;
SELECT t.name, p.type, count(p.type) from CatchedPokemon as c join Pokemon as p on p.id = c.pid join Trainer as t on t.id = c.owner_id group by c.owner_id, p.type;
SELECT t.name, p.name, count(p.name) as Catched from CatchedPokemon as c JOIN Pokemon as p on p.id = c.pid JOIN Trainer as t on t.id = c.owner_id GROUP BY c.owner_id having count(p.name) != count(distinct p.name);
SELECT t.name, g.city from Gym as g join Trainer as t on t.id = g.leader_id where g.leader_id in ( select c.owner_id from CatchedPokemon as c join Pokemon as p on p.id = c.pid join Trainer as t on t.id = c.owner_id group by owner_id having count(distinct p.type)>1);
SELECT t.name, tmp.sum from Gym as g left join (SELECT c.owner_id as ID, sum(c.level) as sum from CatchedPokemon as c join Gym as g on g.leader_id = c.owner_id where c.level >= 50 group by c.owner_id) as tmp on g.leader_id = tmp.ID join Trainer as t on t.id = g.leader_id;
select p.name from (select c.pid as ID from CatchedPokemon as c join Trainer as t on t.id = c.owner_id where t.hometown like 'Sangnok City') as s join (select c.pid as ID from CatchedPokemon as c join Trainer as t on t.id = c.owner_id where t.hometown like 'Blue City') as b on s.ID = b.ID join Pokemon as p on p.id = s.ID;
select p.name from Evolution join Pokemon as p on p.id = after_id where after_id not in (select before_id from Evolution);
