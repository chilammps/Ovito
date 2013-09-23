<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:ng="http://docbook.org/docbook-ng"
	xmlns:db="http://docbook.org/ns/docbook"
	xmlns:exsl="http://exslt.org/common"
	version="1.0"
	exclude-result-prefixes="exsl db ng">

<!-- ================================================================================== -->
<!-- Customizations of of the chunked HTML style for the Qt Assistant help file format. -->
<!-- ================================================================================== -->
	
<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>

<!-- no navigation on pages by default, QtHelp provides its own navigation controls -->
<xsl:param name="suppress.navigation" select="1"/>

<!-- no separate HTML page with index, index is built inside QtHelp pane -->
<xsl:param name="generate.index" select="0"/>

<xsl:param name="use.id.as.filename" select="1"/>
<xsl:param name="chunk.section.depth" select="2"/>
<xsl:param name="chunk.first.sections" select="1"/>
<xsl:param name="manifest.in.base.dir" select="1"/>
<xsl:param name="chapter.autolabel" select="0"/>
<xsl:param name="appendix.autolabel" select="0"/>
<xsl:param name="part.autolabel" select="0"/>
<xsl:param name="section.autolabel" select="0"/>
<xsl:param name="preface.autolabel" select="0"/>
<xsl:param name="toc.section.depth" select="3"/>
	
<xsl:template match="/">
  <!-- * Get a title for current doc so that we let the user -->
  <!-- * know what document we are processing at this point. -->
  <xsl:variable name="doc.title">
    <xsl:call-template name="get.doc.title"/>
  </xsl:variable>
  <xsl:choose>
    <!-- Hack! If someone hands us a DocBook V5.x or DocBook NG document,
         toss the namespace and continue.  Use the docbook5 namespaced
         stylesheets for DocBook5 if you don't want to use this feature.-->
    <!-- include extra test for Xalan quirk -->
    <xsl:when test="(function-available('exsl:node-set') or
                     contains(system-property('xsl:vendor'),
                       'Apache Software Foundation'))
                    and (*/self::ng:* or */self::db:*)">
      <xsl:call-template name="log.message">
        <xsl:with-param name="level">Note</xsl:with-param>
        <xsl:with-param name="source" select="$doc.title"/>
        <xsl:with-param name="context-desc">
          <xsl:text>namesp. cut</xsl:text>
        </xsl:with-param>
        <xsl:with-param name="message">
          <xsl:text>stripped namespace before processing</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:variable name="nons">
        <xsl:apply-templates mode="stripNS"/>
      </xsl:variable>
      <xsl:call-template name="log.message">
        <xsl:with-param name="level">Note</xsl:with-param>
        <xsl:with-param name="source" select="$doc.title"/>
        <xsl:with-param name="context-desc">
          <xsl:text>namesp. cut</xsl:text>
        </xsl:with-param>
        <xsl:with-param name="message">
          <xsl:text>processing stripped document</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:apply-templates select="exsl:node-set($nons)"/>
    </xsl:when>
    <xsl:otherwise>
  <xsl:choose>
    <xsl:when test="$rootid != ''">
      <xsl:choose>
        <xsl:when test="count(key('id',$rootid)) = 0">
          <xsl:message terminate="yes">
            <xsl:text>ID '</xsl:text>
            <xsl:value-of select="$rootid"/>
            <xsl:text>' not found in document.</xsl:text>
          </xsl:message>
        </xsl:when>
        <xsl:otherwise>
          <xsl:if test="$collect.xref.targets = 'yes' or
                        $collect.xref.targets = 'only'">
            <xsl:apply-templates select="key('id', $rootid)"
                        mode="collect.targets"/>
          </xsl:if>
          <xsl:if test="$collect.xref.targets != 'only'">
            <xsl:message>Formatting from <xsl:value-of 
	                          select="$rootid"/></xsl:message>
            <xsl:apply-templates select="key('id',$rootid)"
                        mode="process.root"/>
            <xsl:call-template name="write-qhp"/>          
          </xsl:if>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:if test="$collect.xref.targets = 'yes' or
                    $collect.xref.targets = 'only'">
        <xsl:apply-templates select="/" mode="collect.targets"/>
      </xsl:if>
      <xsl:if test="$collect.xref.targets != 'only'">
        <xsl:apply-templates select="/" mode="process.root"/>
        <xsl:call-template name="write-qhp"/>
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template name="write-qhp">
  <xsl:call-template name="write.chunk">
    <xsl:with-param name="filename">
      <xsl:if test="$manifest.in.base.dir != 0">
        <xsl:value-of select="$base.dir"/>
      </xsl:if>
      <xsl:value-of select="'documentation.qhp'"/>
    </xsl:with-param>
    <xsl:with-param name="method" select="'xml'"/>
    <xsl:with-param name="encoding" select="'utf-8'"/>
    <xsl:with-param name="indent" select="'yes'"/>
    <xsl:with-param name="content">
      <xsl:choose>

        <xsl:when test="$rootid != ''">
          <xsl:variable name="title">            
            <xsl:apply-templates select="key('id',$rootid)" mode="title.markup"/>
          </xsl:variable>
          <xsl:variable name="href">
            <xsl:call-template name="href.target.with.base.dir">
              <xsl:with-param name="object" select="key('id',$rootid)"/>
            </xsl:call-template>
          </xsl:variable>
          
          <QtHelpProject version="1.0">
			<namespace>ovito</namespace>          
			<virtualFolder>doc</virtualFolder>
			<customFilter name="OVITO">
			</customFilter>
			<filterSection>
				<toc>					
					<xsl:apply-templates select="key('id',$rootid)/*" mode="etoc"/>
				</toc>
				<files>					
	    			<xsl:apply-templates select="key('id',$rootid)" mode="enumerate-files-qhp"/>
	    		</files>
				<keywords>
					<xsl:apply-templates select="key('id',$rootid)" mode="enumerate-ids-qhp"/>
				</keywords>
			</filterSection>
          </QtHelpProject>
        </xsl:when>

        <xsl:otherwise>
          <xsl:variable name="title">            
            <xsl:apply-templates select="/*" mode="title.markup"/>
          </xsl:variable>
          <xsl:variable name="href">
            <xsl:call-template name="href.target.with.base.dir">
              <xsl:with-param name="object" select="/"/>
            </xsl:call-template>
          </xsl:variable>
          
          <QtHelpProject version="1.0">
			<namespace>ovito</namespace>
			<virtualFolder>doc</virtualFolder>          
			<customFilter name="OVITO">
			</customFilter>
			<filterSection>
				<toc>					
		            <xsl:apply-templates select="/*/*" mode="etoc"/>
				</toc>
				<files>
					<xsl:apply-templates select="/" mode="enumerate-files-qhp"/>
				</files>
				<keywords>
					<xsl:apply-templates select="/" mode="enumerate-ids-qhp"/>
				</keywords>
			</filterSection>
          </QtHelpProject>
        </xsl:otherwise>

      </xsl:choose>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template match="book|part|reference|preface|chapter|bibliography|appendix|article|glossary|section|sect1|sect2|sect3|sect4|sect5|refentry|colophon|bibliodiv|index" mode="etoc">
  <xsl:variable name="title">
    <xsl:apply-templates select="." mode="title.markup"/>
  </xsl:variable>

  <xsl:variable name="href">
    <xsl:call-template name="href.target.with.base.dir">
      <xsl:with-param name="context" select="/"/>        <!-- Generate links relative to the location of root file/documentation.qhp file -->
    </xsl:call-template>
  </xsl:variable>

  <section title="{normalize-space($title)}" ref="{$href}">
    <xsl:apply-templates select="part|reference|preface|chapter|bibliography|appendix|article|glossary|section|sect1|sect2|sect3|sect4|sect5|refentry|colophon|bibliodiv|index" mode="etoc"/>
  </section>

</xsl:template>

<xsl:template match="text()" mode="etoc"/>

<xsl:template match="set|book|part|preface|chapter|appendix
                     |article
                     |reference|refentry
                     |sect1|sect2|sect3|sect4|sect5
                     |section
                     |book/glossary|article/glossary|part/glossary
                     |book/bibliography|article/bibliography|part/bibliography
                     |colophon"
              mode="enumerate-files-qhp">
  <xsl:variable name="ischunk"><xsl:call-template name="chunk"/></xsl:variable>
  <xsl:if test="$ischunk='1'">
  	<file>
    <xsl:call-template name="make-relative-filename">
      <xsl:with-param name="base.dir">
        <xsl:if test="$manifest.in.base.dir = 0">
          <xsl:value-of select="$base.dir"/>
        </xsl:if>
      </xsl:with-param>
      <xsl:with-param name="base.name">
        <xsl:apply-templates mode="chunk-filename" select="."/>
      </xsl:with-param>
    </xsl:call-template>
    </file>
  </xsl:if>
  <xsl:apply-templates select="*" mode="enumerate-files-qhp"/>
</xsl:template>

<xsl:template match="graphic|imagedata" mode="enumerate-files-qhp">
<file>
<xsl:call-template name="make-relative-filename">
  <xsl:with-param name="base.dir">
    <xsl:if test="$manifest.in.base.dir = 0">
      <xsl:value-of select="$base.dir"/>
    </xsl:if>
  </xsl:with-param>
  <xsl:with-param name="base.name" select="@fileref" />
</xsl:call-template>
</file>
<xsl:apply-templates select="*" mode="enumerate-files-qhp"/>
</xsl:template>

<xsl:template match="text()" mode="enumerate-files-qhp"/>

<xsl:template match="book|part|reference|preface|chapter|bibliography|appendix|article|glossary|section|sect1|sect2|sect3|sect4|sect5|refentry|colophon|bibliodiv|index|variablelist|varlistentry"
              mode="enumerate-ids-qhp">

  <xsl:variable name="id" select="@id" />
              
  <xsl:variable name="href">
    <xsl:call-template name="href.target.with.base.dir">
      <xsl:with-param name="context" select="/"/>        <!-- Generate links relative to the location of root file/documentation.qhp file -->
    </xsl:call-template>
  </xsl:variable>
  
  <xsl:if test="$id">
  	<keyword id="{$id}" ref="{$href}" />
  </xsl:if>
  
  <xsl:apply-templates select="book|part|reference|preface|chapter|bibliography|appendix|article|glossary|section|sect1|sect2|sect3|sect4|sect5|refentry|colophon|bibliodiv|index|variablelist|varlistentry" mode="enumerate-ids-qhp"/>
  
</xsl:template>

<xsl:template match="text()" mode="enumerate-ids-qhp"/>

</xsl:stylesheet>
