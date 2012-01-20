#! /bin/sh
# $Id: genconfig.sh,v 1.51 2011/11/18 11:54:00 nanard Exp $
# miniupnp daemon
# http://miniupnp.free.fr or http://miniupnp.tuxfamily.org/
# (c) 2006-2011 Thomas Bernard
# This software is subject to the conditions detailed in the
# LICENCE file provided within the distribution

RM="rm -f"
CONFIGFILE="config.h"
CONFIGMACRO="__CONFIG_H__"

# version reported in XML descriptions
#UPNP_VERSION=20070827
UPNP_VERSION=`date +"%Y%m%d"`
# Facility to syslog
LOG_MINIUPNPD="LOG_DAEMON"

# detecting the OS name and version
OS_NAME=`uname -s`
OS_VERSION=`uname -r`

# pfSense special case
if [ -f /etc/platform ]; then
	if [ `cat /etc/platform` = "pfSense" ]; then
		OS_NAME=pfSense
		OS_VERSION=`cat /etc/version`
	fi
fi

# OpenWRT special case
if [ -f ./os.openwrt ]; then
	OS_NAME=OpenWRT
	OS_VERSION=$(cat ./os.openwrt)
fi

# Tomato USB special case
if [ -f ../shared/tomato_version ]; then
	OS_NAME=Tomato
	OS_VERSION="Tomato $(cat ../shared/tomato_version)"
fi

${RM} ${CONFIGFILE}

echo "/* MiniUPnP Project" >> ${CONFIGFILE}
echo " * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/" >> ${CONFIGFILE}
echo " * (c) 2006-2011 Thomas Bernard" >> ${CONFIGFILE}
echo " * generated by $0 on `date` */" >> ${CONFIGFILE}
echo "#ifndef $CONFIGMACRO" >> ${CONFIGFILE}
echo "#define $CONFIGMACRO" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}
echo "#include <inttypes.h>" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}
echo "#define MINIUPNPD_VERSION \"`cat VERSION`\"" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}
echo "#define UPNP_VERSION	\"$UPNP_VERSION\"" >> ${CONFIGFILE}

# OS Specific stuff
case $OS_NAME in
	OpenBSD)
		MAJORVER=`echo $OS_VERSION | cut -d. -f1`
		MINORVER=`echo $OS_VERSION | cut -d. -f2`
		#echo "OpenBSD majorversion=$MAJORVER minorversion=$MINORVER"
		# rtableid was introduced in OpenBSD 4.0
		if [ $MAJORVER -ge 4 ]; then
			echo "#define PFRULE_HAS_RTABLEID" >> ${CONFIGFILE}
		fi
		# from the 3.8 version, packets and bytes counters are double : in/out
		if [ \( $MAJORVER -ge 4 \) -o \( $MAJORVER -eq 3 -a $MINORVER -ge 8 \) ]; then
			echo "#define PFRULE_INOUT_COUNTS" >> ${CONFIGFILE}
		fi
		# from the 4.7 version, new pf
		if [ \( $MAJORVER -ge 5 \) -o \( $MAJORVER -eq 4 -a $MINORVER -ge 7 \) ]; then
			echo "#define PF_NEWSTYLE" >> ${CONFIGFILE}
		fi
		# onrdomain was introduced in OpenBSD 5.0
		if [ $MAJORVER -ge 5 ]; then
			echo "#define PFRULE_HAS_ONRDOMAIN" >> ${CONFIGFILE}
		fi
		echo "#define USE_PF 1" >> ${CONFIGFILE}
		FW=pf
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		OS_URL=http://www.openbsd.org/
		;;
	FreeBSD)
		VER=`grep '#define __FreeBSD_version' /usr/include/sys/param.h | awk '{print $3}'`
		if [ $VER -ge 700049 ]; then
			echo "#define PFRULE_INOUT_COUNTS" >> ${CONFIGFILE}
		fi
		# new way to see which one to use PF or IPF.
		# see http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=957
		# source file with handy subroutines like checkyesno
		. /etc/rc.subr
		# source config file so we can probe vars
		. /etc/rc.conf
		if checkyesno ipfilter_enable; then
			echo "Using ipf"
			FW=ipf
			echo "#define USE_IPF 1" >> ${CONFIGFILE}
		elif checkyesno pf_enable; then
			echo "Using pf"
			FW=pf
			echo "#define USE_PF 1" >> ${CONFIGFILE}
		# TODO : Add support for IPFW
		# echo "#define USE_IPFW 1" >> ${CONFIGFILE}
		# FW=ipfw
		else
			echo "Could not detect usage of ipf or pf. Compiling for pf by default"
			FW=pf
			echo "#define USE_PF 1" >> ${CONFIGFILE}
		fi
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		OS_URL=http://www.freebsd.org/
		;;
	pfSense)
		# we need to detect if PFRULE_INOUT_COUNTS macro is needed
		echo "#define USE_PF 1" >> ${CONFIGFILE}
		FW=pf
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		OS_URL=http://www.pfsense.com/
		;;
	NetBSD)
		# source file with handy subroutines like checkyesno
		. /etc/rc.subr
		# source config file so we can probe vars
		. /etc/rc.conf
		if checkyesno pf; then
			echo "#define USE_PF 1" >> ${CONFIGFILE}
			FW=pf
		elif checkyesno ipfilter; then
			echo "#define USE_IPF 1" >> ${CONFIGFILE}
			FW=ipf
		else
			echo "Could not detect ipf nor pf, defaulting to pf."
			echo "#define USE_PF 1" >> ${CONFIGFILE}
			FW=pf
		fi
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		OS_URL=http://www.netbsd.org/
		;;
	DragonFly)
		# source file with handy subroutines like checkyesno
		. /etc/rc.subr
		# source config file so we can probe vars
		. /etc/rc.conf
		if checkyesno pf; then
			echo "#define USE_PF 1" >> ${CONFIGFILE}
			FW=pf
		elif checkyesno ipfilter; then
			echo "#define USE_IPF 1" >> ${CONFIGFILE}
			FW=ipf
		else
			echo "Could not detect ipf nor pf, defaulting to pf."
			echo "#define USE_PF 1" >> ${CONFIGFILE}
			FW=pf
		fi
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		OS_URL=http://www.dragonflybsd.org/
		;;
	SunOS)
		echo "#define USE_IPF 1" >> ${CONFIGFILE}
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		FW=ipf
		echo "#define LOG_PERROR 0" >> ${CONFIGFILE}
		echo "#define SOLARIS_KSTATS 1" >> ${CONFIGFILE}
		OS_URL=http://www.sun.com/solaris/
		;;
	Linux)
		OS_URL=http://www.kernel.org/
		KERNVERA=`echo $OS_VERSION | awk -F. '{print $1}'`
		KERNVERB=`echo $OS_VERSION | awk -F. '{print $2}'`
		KERNVERC=`echo $OS_VERSION | awk -F. '{print $3}'`
		KERNVERD=`echo $OS_VERSION | awk -F. '{print $4}'`
		#echo "$KERNVERA.$KERNVERB.$KERNVERC.$KERNVERD"
		# Debian GNU/Linux special case
		if [ -f /etc/debian_version ]; then
			OS_NAME=Debian
			OS_VERSION=`cat /etc/debian_version`
			OS_URL=http://www.debian.org/
		fi
		# same thing for Gentoo linux
		if  [ -f /etc/gentoo-release ]; then
			OS_NAME=Gentoo
			OS_VERSION=`cat /etc/gentoo-release`
			OS_URL=http://www.gentoo.org/
		fi
		# use lsb_release (Linux Standard Base) when available
		LSB_RELEASE=`which lsb_release`
		if [ 0 -eq $? ]; then
			OS_NAME=`${LSB_RELEASE} -i -s`
			OS_VERSION=`${LSB_RELEASE} -r -s`
			case $OS_NAME in
				Debian)
					OS_URL=http://www.debian.org/
					OS_VERSION=`${LSB_RELEASE} -c -s`
					;;
				Ubuntu)
					OS_URL=http://www.ubuntu.com/
					OS_VERSION=`${LSB_RELEASE} -c -s`
					;;
				Gentoo)
					OS_URL=http://www.gentoo.org/
					;;
			esac
		fi
		echo "#define USE_NETFILTER 1" >> ${CONFIGFILE}
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		FW=netfilter
		;;
	OpenWRT)
		OS_URL=http://www.openwrt.org/
		echo "#define USE_NETFILTER 1" >> ${CONFIGFILE}
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		FW=netfilter
		;;
	Tomato)
		OS_NAME=UPnP
		OS_URL=http://tomatousb.org/
		echo "" >> ${CONFIGFILE}
		echo "#include <tomato_config.h>" >> ${CONFIGFILE}
		echo "" >> ${CONFIGFILE}
		echo "#define USE_NETFILTER 1" >> ${CONFIGFILE}
		echo "#ifdef LINUX26" >> ${CONFIGFILE}
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		echo "#endif" >> ${CONFIGFILE}
		echo "#ifdef TCONFIG_IPV6" >> ${CONFIGFILE}
		echo "#define ENABLE_IPV6" >> ${CONFIGFILE}
		echo "#endif" >> ${CONFIGFILE}
		FW=netfilter
		;;
	Darwin)
		echo "#define USE_IPFW 1" >> ${CONFIGFILE}
		echo "#define USE_IFACEWATCHER 1" >> ${CONFIGFILE}
		FW=ipfw
		OS_URL=http://developer.apple.com/macosx
		;;
	*)
		echo "Unknown OS : $OS_NAME"
		echo "Please contact the author at http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/."
		exit 1
		;;
esac

echo "Configuring compilation for [$OS_NAME] [$OS_VERSION] with [$FW] firewall software."
echo "Please edit config.h for more compilation options."

# define SUPPORT_REMOTEHOST if the FW related code really supports setting
# a RemoteHost
if [ \( "$FW" = "netfilter" \) -o \( "$FW" = "pf" \) -o \( "$FW" = "ipfw" \) ] ; then
	echo "#define SUPPORT_REMOTEHOST" >> ${CONFIGFILE}
fi

echo "" >> ${CONFIGFILE}
echo "#define OS_NAME		\"$OS_NAME\"" >> ${CONFIGFILE}
echo "#define OS_VERSION	\"$OS_NAME/$OS_VERSION\"" >> ${CONFIGFILE}
echo "#define OS_URL		\"${OS_URL}\"" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* syslog facility to be used by miniupnpd */" >> ${CONFIGFILE}
echo "#define LOG_MINIUPNPD		 ${LOG_MINIUPNPD}" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Uncomment the following line to allow miniupnpd to be" >> ${CONFIGFILE}
echo " * controlled by miniupnpdctl */" >> ${CONFIGFILE}
echo "/*#define USE_MINIUPNPDCTL*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Comment the following line to disable NAT-PMP operations */" >> ${CONFIGFILE}
echo "#define ENABLE_NATPMP" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Uncomment the following line to enable generation of" >> ${CONFIGFILE}
echo " * filter rules with pf */" >> ${CONFIGFILE}
echo "/*#define PF_ENABLE_FILTER_RULES*/">> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Uncomment the following line to enable caching of results of" >> ${CONFIGFILE}
echo " * the getifstats() function */" >> ${CONFIGFILE}
echo "/*#define ENABLE_GETIFSTATS_CACHING*/" >> ${CONFIGFILE}
echo "/* The cache duration is indicated in seconds */" >> ${CONFIGFILE}
echo "#define GETIFSTATS_CACHING_DURATION 2" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Uncomment the following line to enable multiple external ip support */" >> ${CONFIGFILE}
echo "/* note : That is EXPERIMENTAL, do not use that unless you know perfectly what you are doing */" >> ${CONFIGFILE}
echo "/* Dynamic external ip adresses are not supported when this option is enabled." >> ${CONFIGFILE}
echo " * Also note that you would need to configure your .conf file accordingly. */" >> ${CONFIGFILE}
echo "/*#define MULTIPLE_EXTERNAL_IP*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Comment the following line to use home made daemonize() func instead" >> ${CONFIGFILE}
echo " * of BSD daemon() */" >> ${CONFIGFILE}
echo "#define USE_DAEMON" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Uncomment the following line to enable lease file support */" >> ${CONFIGFILE}
echo "/*#define ENABLE_LEASEFILE*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Define one or none of the two following macros in order to make some" >> ${CONFIGFILE}
echo " * clients happy. It will change the XML Root Description of the IGD." >> ${CONFIGFILE}
echo " * Enabling the Layer3Forwarding Service seems to be the more compatible" >> ${CONFIGFILE}
echo " * option. */" >> ${CONFIGFILE}
echo "/*#define HAS_DUMMY_SERVICE*/" >> ${CONFIGFILE}
echo "#define ENABLE_L3F_SERVICE" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Enable IP v6 support */" >> ${CONFIGFILE}
echo "/*#define ENABLE_IPV6*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Enable the support of IGD v2 specification." >> ${CONFIGFILE}
echo " * This is not fully tested yet and can cause incompatibilities with some" >> ${CONFIGFILE}
echo " * control points, so enable with care. */" >> ${CONFIGFILE}
echo "/*#define IGD_V2*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "#ifdef IGD_V2" >> ${CONFIGFILE}
echo "/* Enable DeviceProtection service (IGDv2) */" >> ${CONFIGFILE}
echo "#define ENABLE_DP_SERVICE" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}
echo "/* Enable WANIPv6FirewallControl service (IGDv2). needs IPv6 */" >> ${CONFIGFILE}
echo "#ifdef ENABLE_IPV6" >> ${CONFIGFILE}
echo "#define ENABLE_6FC_SERVICE" >> ${CONFIGFILE}
echo "#endif /* ENABLE_IPV6 */" >> ${CONFIGFILE}
echo "#endif /* IGD_V2 */" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* UPnP Events support. Working well enough to be enabled by default." >> ${CONFIGFILE}
echo " * It can be disabled to save a few bytes. */" >> ${CONFIGFILE}
echo "#define ENABLE_EVENTS" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* include interface name in pf and ipf rules */" >> ${CONFIGFILE}
echo "#define USE_IFNAME_IN_RULES" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Experimental NFQUEUE support. */" >> ${CONFIGFILE}
echo "/*#define ENABLE_NFQUEUE*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "/* Enable to make MiniUPnPd more strict about UPnP conformance" >> ${CONFIGFILE}
echo " * and the messages it receives from control points */" >> ${CONFIGFILE}
echo "/*#define UPNP_STRICT*/" >> ${CONFIGFILE}
echo "" >> ${CONFIGFILE}

echo "#endif" >> ${CONFIGFILE}

exit 0
