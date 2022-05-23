CREATE TABLE IF NOT EXISTS `account_bound_mount` (
    `accountid` int(10) unsigned NOT NULL,
    `spellid` mediumint(8) unsigned NOT NULL,
    `allowablerace` int(10) NOT NULL,
    `allowableclass` int(10) NOT NULL,
    `requiredlevel` tinyint(3) unsigned NOT NULL,
    `requiredskill` smallint(3) unsigned NOT NULL,
    `requiredskillrank` smallint(3) unsigned NOT NULL,
    PRIMARY KEY (`accountid`,`spellid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
