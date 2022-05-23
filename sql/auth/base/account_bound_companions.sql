CREATE TABLE IF NOT EXISTS `account_bound_companions` (
	`accountid` INT(10) UNSIGNED NOT NULL,
	`spellid` MEDIUMINT(8) UNSIGNED NOT NULL,
	`allowablerace` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`accountid`, `spellid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;