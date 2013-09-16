<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:ng="http://docbook.org/docbook-ng"
	xmlns:db="http://docbook.org/ns/docbook"
	xmlns:exsl="http://exslt.org/common"
	version="1.0"
	exclude-result-prefixes="exsl db ng">

<!-- ================================================================================== -->
<!-- Customizations of of the chunked HTML style for the generation of the HTML formatted manual. -->
<!-- ================================================================================== -->
	
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>

<xsl:param name="use.id.as.filename" select="1"/>
<xsl:param name="chunk.section.depth" select="2"/>
<xsl:param name="chunk.first.sections" select="0"/>
<xsl:param name="html.stylesheet" select="'manual.css'"/>
<xsl:param name="section.autolabel" select="1"/>
<xsl:param name="section.label.includes.component.label" select="1"/>
<xsl:param name="xref.with.number.and.title" select="0"/>

</xsl:stylesheet>
