CREATE TABLE IF NOT EXISTS `account_bound_mounts` (
    `accountid` INT(10) UNSIGNED NOT NULL,
    `spellid` MEDIUMINT(8) UNSIGNED NOT NULL,
    `allowablerace` INT(11) NOT NULL,
    `allowableclass` INT(11) NOT NULL,
    `requiredlevel` TINYINT(3) UNSIGNED NOT NULL,
    `requiredskill` SMALLINT(3) UNSIGNED NOT NULL,
    `requiredskillrank` SMALLINT(3) UNSIGNED NOT NULL,
    PRIMARY KEY (`accountid`,`spellid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
