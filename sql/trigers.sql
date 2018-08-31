CREATE
DEFINER = 'root'@'%'
TRIGGER NodeInsert
AFTER INSERT
ON departments
FOR EACH ROW
BEGIN
  INSERT INTO tree
  SET id = new.id,
      parent_id = new.id,
      level = 0;
  INSERT INTO tree
    SELECT
      n.id,
      t.parent_id,
      t.level + 1
    FROM (SELECT
             id,
             parent_id
           FROM departments
           WHERE id = new.id) n,
         tree t
    WHERE n.parent_id = t.id;
END
$$
CREATE
DEFINER = 'root'@'%'
TRIGGER NodeUpdate
AFTER UPDATE
ON departments
FOR EACH ROW
BEGIN
  IF NEW.parent_id != OLD.parent_id THEN
    DROP TEMPORARY TABLE IF EXISTS child;
    CREATE TEMPORARY TABLE child (
      id int,
      level int
    );
    INSERT INTO child
      SELECT
        t.id,
        t.level
      FROM tree t
      WHERE new.id = t.parent_id
      AND t.level > 0;
    DELETE
      FROM tree
    WHERE id IN (SELECT
          id
        FROM child)
      AND parent_id IN (SELECT
          t.parent_id
        FROM (SELECT
            *
          FROM tree) AS t
        WHERE new.id = t.id
        AND t.level > 0);
    DELETE
      FROM tree
    WHERE id = NEW.id
      AND level > 0;
    INSERT INTO tree
      SELECT
        n.id,
        t.parent_id,
        t.level + 1
      FROM (SELECT
               id,
               parent_id
             FROM departments
             WHERE id = new.id) n,
           tree t
      WHERE n.parent_id = t.id;
    INSERT INTO tree
      SELECT
        c.id,
        t.parent_id,
        t.level + c.level
      FROM tree t,
           child c
      WHERE new.id = t.id
      AND t.level > 0;
    DROP TEMPORARY TABLE child;
  END IF;
END
$$
CREATE
DEFINER = 'root'@'%'
TRIGGER NodeInsertPo
AFTER INSERT
ON po
FOR EACH ROW
BEGIN
  INSERT INTO potree
  SET id = new.CodPO,
      parent_id = new.CodPO,
      level = 0;
  INSERT INTO potree
    SELECT
      n.CodPO,
      t.parent_id,
      t.level + 1
    FROM (SELECT
             CodPO,
             CodGroupPO
           FROM po
           WHERE CodPO = new.CodPO) n,
         potree t
    WHERE n.CodGroupPO = t.id;
END
$$
CREATE
DEFINER = 'root'@'%'
TRIGGER NodeUpdatePo
AFTER UPDATE
ON po
FOR EACH ROW
BEGIN
  IF NEW.CodGroupPO != OLD.CodGroupPO THEN
    DROP TEMPORARY TABLE IF EXISTS childpo;
    CREATE TEMPORARY TABLE childpo (
      id int,
      level int
    );
    INSERT INTO childpo
      SELECT
        t.id,
        t.level
      FROM potree t
      WHERE new.CodPO = t.parent_id
      AND t.level > 0;
    DELETE
      FROM potree
    WHERE id IN (SELECT
          id
        FROM childpo)
      AND parent_id IN (SELECT
          t.parent_id
        FROM (SELECT
            *
          FROM potree) AS t
        WHERE new.CodPO = t.id
        AND t.level > 0);
    DELETE
      FROM potree
    WHERE id = NEW.CodPO
      AND level > 0;
    INSERT INTO potree
      SELECT
        n.CodPO,
        t.parent_id,
        t.level + 1
      FROM (SELECT
               CodPO,
               CodGroupPO
             FROM po
             WHERE CodPO = new.CodPO) n,
           potree t
      WHERE n.CodGroupPO = t.id;
    INSERT INTO potree
      SELECT
        c.id,
        t.parent_id,
        t.level + c.level
      FROM potree t,
           childpo c
      WHERE new.CodPO = t.id
      AND t.level > 0;
    DROP TEMPORARY TABLE childpo;
  END IF;
END
$$
CREATE
DEFINER = 'root'@'%'
TRIGGER NodeInsertDevice
AFTER INSERT
ON device
FOR EACH ROW
BEGIN
  INSERT INTO devtree
  SET id = new.id,
      parent_id = new.id,
      level = 0;
  INSERT INTO devtree
    SELECT
      n.id,
      t.parent_id,
      t.level + 1
    FROM (SELECT
             id,
             parent_id
           FROM device
           WHERE id = new.id) n,
         devtree t
    WHERE n.parent_id = t.id;
END
$$
CREATE
DEFINER = 'root'@'%'
TRIGGER NodeUpdateDevice
AFTER UPDATE
ON device
FOR EACH ROW
BEGIN
  IF NEW.parent_id != OLD.parent_id THEN
    DROP TEMPORARY TABLE IF EXISTS childdev;
    CREATE TEMPORARY TABLE childdev (
      id int,
      level int
    );
    INSERT INTO childdev
      SELECT
        t.id,
        t.level
      FROM devtree t
      WHERE new.id = t.parent_id
      AND t.level > 0;
    DELETE
      FROM devtree
    WHERE id IN (SELECT
          id
        FROM childdev)
      AND parent_id IN (SELECT
          t.parent_id
        FROM (SELECT
            *
          FROM devtree) AS t
        WHERE new.id = t.id
        AND t.level > 0);
    DELETE
      FROM devtree
    WHERE id = NEW.id
      AND level > 0;
    INSERT INTO devtree
      SELECT
        n.id,
        t.parent_id,
        t.level + 1
      FROM (SELECT
               id,
               parent_id
             FROM device
             WHERE id = new.id) n,
           devtree t
      WHERE n.parent_id = t.id;
    INSERT INTO devtree
      SELECT
        c.id,
        t.parent_id,
        t.level + c.level
      FROM devtree t,
           childdev c
      WHERE new.id = t.id
      AND t.level > 0;
    DROP TEMPORARY TABLE childdev;
  END IF;
END