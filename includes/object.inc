<?php
/**
 * @file
 * Implementation of classes from the batch ingest framework for thesis files.
 */

/**
 * Batch preprocessor.
 */
class ThesisBatchObject extends IslandoraScanBatch {
  /**
   * Function batchProcess.
   */
  public function batchProcess() {
    $this->label = $this->getTitle();
    $this->getMods();
    $this->getDc();

    if (!isset($this['OBJ'])) {
      $other = array_diff_key($this->objectInfo, array_flip($this->objectInfoDatastreamExclusions));
      foreach ($other as $obj) {
        list($dsid, $mimetype) = static::determineDSIDAndMimetype($obj->filename);
        $obj_datastream = $this->constructDatastream($dsid);
        $obj_datastream->mimetype = $mimetype;
        $obj_datastream->label = "$dsid Datastream";
        $obj_datastream->setContentFromFile($obj->uri, FALSE);
        $this->ingestDatastream($obj_datastream);
      }
    }
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Function addRelationships.
   */
  public function addRelationships() {
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'ir:thesisCModel');
    $this->models = 'ir:thesisCModel';
  }

}
