-- --------------------------------------------------------
-- 主机:                           192.168.80.129
-- 服务器版本:                        5.5.41-MariaDB - MariaDB Server
-- 服务器操作系统:                      Linux
-- HeidiSQL 版本:                  9.4.0.5142
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- 导出 G2652 的数据库结构
USE `G2652`;

-- 数据导出被取消选择。
-- 导出  表 G2652.files 结构
DROP TABLE IF EXISTS `files`;
CREATE TABLE IF NOT EXISTS `files` (
  `uid` int(11) NOT NULL,
  `filename` varchar(56) NOT NULL,
  `len` int(11) DEFAULT NULL,
  `path` varchar(128) NOT NULL,
  `md5` char(32) NOT NULL DEFAULT '',
  `complete` tinyint(1) DEFAULT NULL,
  `modtime` datetime DEFAULT NULL,
  `chunks` mediumtext,
  PRIMARY KEY (`uid`,`filename`,`path`),
  CONSTRAINT `files_ibfk_1` FOREIGN KEY (`uid`) REFERENCES `user` (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 数据导出被取消选择。
-- 导出  表 G2652.inode 结构
DROP TABLE IF EXISTS `inode`;
CREATE TABLE IF NOT EXISTS `inode` (
  `md5` char(32) NOT NULL DEFAULT '',
  `refcount` int(11) DEFAULT NULL,
  PRIMARY KEY (`md5`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 数据导出被取消选择。
-- 导出  表 G2652.journal 结构
DROP TABLE IF EXISTS `journal`;
CREATE TABLE IF NOT EXISTS `journal` (
  `uid` int(11) DEFAULT NULL,
  `filename` varchar(256) DEFAULT NULL,
  `path` varchar(256) DEFAULT NULL,
  `md5` char(32) DEFAULT NULL,
  `len` int(11) DEFAULT NULL,
  `action` enum('add','delete') DEFAULT NULL,
  `modtime` datetime DEFAULT NULL,
  `commitid` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`commitid`),
  KEY `uid` (`uid`),
  CONSTRAINT `journal_ibfk_1` FOREIGN KEY (`uid`) REFERENCES `user` (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=744 DEFAULT CHARSET=utf8;

-- 数据导出被取消选择。
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
